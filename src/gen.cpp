const char* help =
	"gen.cpp: data generation for anomaly detection\n\n"

	"This program generates an raw data and packs it in binary form.\n"
	"Data is not featurized at this stage, so that we may try several\n"
	"methods of featurization fromthe same raw data.";

#include <memory>

#include "pack.hpp"

#define AD_FLAG_OUTPUT "-o"
#define AD_FLAG_HELP "-h"


int main(int arg_count, char** args) {
	// https://www.desmos.com/calculator/nui2htn4wf
	//
	// Simple 2D points that should map to two clusters
	float data_set [8][2] = {
		{ 1.f, 1.f },
		{ .9f, .9f },
		{ .8f, 1.f },
		{ 1.f, .75f },

		{ -1.f, -1.f },
		{ -.9f, -.9f },
		{ -.75f, -.9f },
		{ -.9f, -.75f },
	};
	int32 buffer_size = 1024 * 1024;

	// Command line args
	char output_path [AD_PATH_SIZE] = { 0 };

	for (int32 i = 1; i < arg_count; i++) {
		char* flag = args[i];
		if (!strcmp(flag, AD_FLAG_OUTPUT)) {
			char* arg = args[++i];
			strncpy(output_path, arg, AD_PATH_SIZE);
		}
		else if (!strcmp(flag, AD_FLAG_HELP)) {
			printf("%s\n", help);
			exit(0);
		}
	}
	
	// Sanity checks
	if (!strlen(output_path)) {
		printf("%s\n", help);
		exit(1);
	}

	ad_pack_context context;
	char* buffer = (char*)calloc(sizeof(char), buffer_size);
	pack_ctx_init(&context, buffer, buffer_size);

	for (int i = 0; i < 8; i++) {
		pack_ctx_row(&context);
		pack_ctx_float32(&context, data_set[i][0]);
		pack_ctx_float32(&context, data_set[i][1]);
	}
	pack_ctx_end(&context);

	pack_ctx_write(&context, output_path);
	exit(0);
}



#if 0
#include <stdio.h>
#include <fcntl.h>
#include <stdlib.h>
#include <sys/ioctl.h>     // for _IOW, a macro required by FSEVENTS_CLONE
#include <sys/types.h>     // for uint32_t and friends, on which fsevents.h relies
#include <unistd.h>
#include <string.h> // memset
#include <sys/_types.h>     // for uint32_t and friends, on which fsevents.h relies

#include <sys/stat.h> // for mkdir
#include <libgen.h> // for basename
#include <sys/sysctl.h> // for sysctl, KERN_PROC, etc.
#include <errno.h>

#include "fsevents.h"
#include <signal.h> // for kill

typedef struct kfs_event_a {
  uint16_t type;
  uint16_t refcount;
  pid_t    pid;
} kfs_event_a;

typedef struct kfs_event_arg {
  uint16_t type;
  uint16_t pathlen;
  char data[0];
} kfs_event_arg;


static char *
getProcName(long pid)
{

  static char procName[4096];
  size_t len = 1000;
  int rc;
  int mib[4];

  // minor optimization
  if (pid != lastPID) {
    memset(procName, '\0', 4096);

    mib[0] = CTL_KERN;
    mib[1] = KERN_PROC;
    mib[2] = KERN_PROC_PID;
    mib[3] = pid;

    if ((rc = sysctl(mib, 4, procName, &len, NULL,0)) < 0) {
        perror("trace facility failure, KERN_PROC_PID\n");
        exit(1);
    }

   }
    return (((struct kinfo_proc *)procName)->kp_proc.p_comm);
}


int
main (int argc, char **argv)
{
    
    int i;
    int rc;
    fsevent_clone_args  clone_args;
    unsigned short *arg_type;
    char *buf = malloc(BUFSIZE);
    int8_t events [FSE_MAX_EVENTS] = {0};
    
    if (geteuid()) {
        fprintf(stderr,"Opening /dev/fsevents requires root permissions\n");
    }

    int fsed = open("/dev/fsevents", O_RDONLY);
    if (fsed < 0) {
        perror ("open");
        exit(1);
    }

    events[FSE_CREATE_FILE]      = FSE_REPORT;
    events[FSE_DELETE]           = FSE_REPORT;
    events[FSE_RENAME]           = FSE_REPORT;
    events[FSE_CONTENT_MODIFIED] = FSE_REPORT;
    events[FSE_CREATE_DIR]       = FSE_REPORT;
    events[FSE_CHOWN]            = FSE_REPORT;

    int cloned_fsed;
    fsevent_clone_args clone_args = { 0 };
    clone_args.fd = &cloned_fsed;
    clone_args.event_queue_depth = 100;
    clone_args.event_list = events;
    clone_args.num_events = FSE_MAX_EVENTS;
    
    int rc = ioctl(fsed, FSEVENTS_CLONE, &clone_args);
    
    if (rc < 0) {
        perror("ioctl");
        exit(2);
    }
    
    close (fsed);

    while ((rc = read(cloned_fsed, buf, BUFSIZE)) > 0)
    {
        // rc returns the count of bytes for one or more events:
        int offInBuf = 0;

        while (offInBuf < rc) {
           struct kfs_event_a* fse = (struct kfs_event_a*)(buf + offInBuf);
           struct kfs_event_arg* fse_arg;

            if (fse->type == FSE_EVENTS_DROPPED)
            {
                printf("Some events dropped\n");
                break;
            }

           int print = 0;
           char *procName = getProcName(fse->pid);
           offInBuf+= sizeof(struct kfs_event_a);
           fse_arg = (struct kfs_event_arg *) &buf[offInBuf];

           if (interesting_process(fse->pid, procFilters, numProcFilters)
                    && interesting_file (fse_arg->data, fileFilters, numFileFilters))
           {
             
            // Fix: Don't autolink own files
              if (fse->type == FSE_CREATE_FILE)

            {
               int fileLen = strlen(fse_arg->data);
            
               char *linkName = malloc (fileLen + 20);
#ifndef ARM
               strcpy(linkName, fse_arg->data);
#else
            
               if (strncmp(fse_arg->data, "/private/var",12) == 0)
                {
                 // Might be in some subdir which will go away
                 strcpy(linkName, filemonDir);
                 strcat(linkName,"/");
                 strcat (linkName, basename(fse_arg->data));

                }
#endif
               
                 // Don't need to worry about buffer boundaries here..
               
               snprintf(linkName + strlen(linkName), fileLen + 20, ".filemon.%d", autolink);
               int rc = link (fse_arg->data, linkName);
               if (rc) { fprintf(stderr,"%sWarning: Unable to autolink %s%s - file must have been deleted already\n",
                    color ? RED : "",
                    fse_arg->data,
                    color ? NORMAL : "");}

               else    { fprintf(stderr,"%sAuto-linked %s to %s%s\n",
                    color ? GREEN : "",
                    fse_arg->data, linkName,
                    color ? NORMAL :"");
                       autolink++;
                   }

               free (linkName);
            }


               offInBuf += sizeof(kfs_event_arg) + fse_arg->pathlen ;

           int arg_len = doArg(buf + offInBuf,print);
               offInBuf += arg_len;
           while (arg_len >2 && offInBuf < rc)
            {
                   arg_len = doArg(buf + offInBuf, print);
                       offInBuf += arg_len;
            }
    
        }
           memset (buf,'\0', BUFSIZE);
        if (rc > offInBuf) { printf ("*** Warning: Some events may be lost\n"); }
    }

} // end main
#endif
