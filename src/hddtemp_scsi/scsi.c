/*
 * Copyright (C) 2002  Emmanuel VARAGNAT <hddtemp@guzu.net>
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, see <http://www.gnu.org/licenses/>.
 */
/* 
 * Adapted from a patch sent by: Frederic LOCHON <lochon@roulaise.net>
 */
/*
 * Adapted for use with nwipe by: Gerold Gruber <Gerold.Gruber@edv2g.de>
 */


// Include file generated by ./configure
#ifdef HAVE_CONFIG_H
#include <config.h>
#endif

// Gettext includes
#if ENABLE_NLS
#include <libintl.h>
#define _(String) gettext (String)
#else
#define _(String) (String)
#endif

// Standard includes
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <string.h>
#include <fcntl.h>
#include <errno.h>
#include <sys/ioctl.h>
#include <scsi/scsi.h>

// Application specific includes
#include "scsicmds.h"
#include "hddtemp.h"

int scsi_get_temperature(struct disk *dsk) {
  int              i;
  int              tempPage = 0;
  unsigned char    buffer[1024];

  /*
    on triche un peu
    we cheat a little and do not really read form drivedb as SCSI disks are not included there
    original code omitted as there is no need for it in the context of nwipe
  */

  /*
    Enable SMART
  */
  if (scsi_smartDEXCPTdisable(dsk->fd) != 0) {
    snprintf(dsk->errormsg, MAX_ERRORMSG_SIZE, "%s", strerror(errno));
    close(dsk->fd);
    dsk->fd = -1;
    return GETTEMP_ERROR;
  }

  /*
    Temp. capable 
  */  
  if (scsi_logsense(dsk->fd , SUPPORT_LOG_PAGES, buffer, sizeof(buffer)) != 0) {
    snprintf(dsk->errormsg, MAX_ERRORMSG_SIZE, _("log sense failed : %s"), strerror(errno));
    close(dsk->fd);
    dsk->fd = -1;
    return GETTEMP_ERROR;
   }

   for ( i = 4; i < buffer[3] + LOGPAGEHDRSIZE ; i++) {
     if (buffer[i] == TEMPERATURE_PAGE) {
       tempPage = 1;
       break;
     }
   }

   if(tempPage) {
      /* 
	 get temperature (from scsiGetTemp (scsicmd.c))
      */
      if (scsi_logsense(dsk->fd , TEMPERATURE_PAGE, buffer, sizeof(buffer)) != 0) {
	snprintf(dsk->errormsg, MAX_ERRORMSG_SIZE, _("log sense failed : %s"), strerror(errno));
	close(dsk->fd);
	dsk->fd = -1;
	return GETTEMP_ERROR;
      }

      if( (int)buffer[7] == 2 ) /* PARAMETER LENGTH */
      {
          dsk->value = buffer[9];
      }
      else
      {
          snprintf(dsk->errormsg, MAX_ERRORMSG_SIZE, _("parameter length unexpected: %d"), (int)buffer[7] );
          return GETTEMP_UNKNOWN;
      }
      dsk->refvalue = buffer[15];
      if( (int)buffer[13] == 2 ) /* PARAMETER LENGTH */
      {
          dsk->refvalue = buffer[15];
      }
      else
      {
          snprintf(dsk->errormsg, MAX_ERRORMSG_SIZE, _("parameter ref length unexpected: %d"), (int)buffer[13] );
          return GETTEMP_UNKNOWN;
      }
      return GETTEMP_SUCCESS;
   } else {
     return GETTEMP_NOSENSOR;
   }
}
