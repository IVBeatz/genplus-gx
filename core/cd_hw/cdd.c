/***************************************************************************************
 *  Genesis Plus
 *  CD drive processor & CD-DA fader
 *
 *  Copyright (C) 2012-2015  Eke-Eke (Genesis Plus GX)
 *
 *  Redistribution and use of this code or any derivative works are permitted
 *  provided that the following conditions are met:
 *
 *   - Redistributions may not be sold, nor may they be used in a commercial
 *     product or activity.
 *
 *   - Redistributions that are modified from the original source must include the
 *     complete source code, including the source code for all components used by a
 *     binary built from the modified sources. However, as a special exception, the
 *     source code distributed need not include anything that is normally distributed
 *     (in either source or binary form) with the major components (compiler, kernel,
 *     and so on) of the operating system on which the executable runs, unless that
 *     component itself accompanies the executable.
 *
 *   - Redistributions must reproduce the above copyright notice, this list of
 *     conditions and the following disclaimer in the documentation and/or other
 *     materials provided with the distribution.
 *
 *  THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS"
 *  AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
 *  IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE
 *  ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT OWNER OR CONTRIBUTORS BE
 *  LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR
 *  CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF
 *  SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS
 *  INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN
 *  CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE)
 *  ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE
 *  POSSIBILITY OF SUCH DAMAGE.
 *
 ****************************************************************************************/
#include "shared.h"

#if defined(USE_LIBTREMOR) || defined(USE_LIBVORBIS)
#define SUPPORTED_EXT 20
#else
#define SUPPORTED_EXT 10
#endif

/* BCD conversion lookup tables */
static const uint8 lut_BCD_8[100] =
{
  0x00, 0x01, 0x02, 0x03, 0x04, 0x05, 0x06, 0x07, 0x08, 0x09, 
  0x10, 0x11, 0x12, 0x13, 0x14, 0x15, 0x16, 0x17, 0x18, 0x19, 
  0x20, 0x21, 0x22, 0x23, 0x24, 0x25, 0x26, 0x27, 0x28, 0x29, 
  0x30, 0x31, 0x32, 0x33, 0x34, 0x35, 0x36, 0x37, 0x38, 0x39, 
  0x40, 0x41, 0x42, 0x43, 0x44, 0x45, 0x46, 0x47, 0x48, 0x49, 
  0x50, 0x51, 0x52, 0x53, 0x54, 0x55, 0x56, 0x57, 0x58, 0x59, 
  0x60, 0x61, 0x62, 0x63, 0x64, 0x65, 0x66, 0x67, 0x68, 0x69, 
  0x70, 0x71, 0x72, 0x73, 0x74, 0x75, 0x76, 0x77, 0x78, 0x79, 
  0x80, 0x81, 0x82, 0x83, 0x84, 0x85, 0x86, 0x87, 0x88, 0x89, 
  0x90, 0x91, 0x92, 0x93, 0x94, 0x95, 0x96, 0x97, 0x98, 0x99, 
};

static const uint16 lut_BCD_16[100] =
{
  0x0000, 0x0001, 0x0002, 0x0003, 0x0004, 0x0005, 0x0006, 0x0007, 0x0008, 0x0009, 
  0x0100, 0x0101, 0x0102, 0x0103, 0x0104, 0x0105, 0x0106, 0x0107, 0x0108, 0x0109, 
  0x0200, 0x0201, 0x0202, 0x0203, 0x0204, 0x0205, 0x0206, 0x0207, 0x0208, 0x0209, 
  0x0300, 0x0301, 0x0302, 0x0303, 0x0304, 0x0305, 0x0306, 0x0307, 0x0308, 0x0309, 
  0x0400, 0x0401, 0x0402, 0x0403, 0x0404, 0x0405, 0x0406, 0x0407, 0x0408, 0x0409, 
  0x0500, 0x0501, 0x0502, 0x0503, 0x0504, 0x0505, 0x0506, 0x0507, 0x0508, 0x0509, 
  0x0600, 0x0601, 0x0602, 0x0603, 0x0604, 0x0605, 0x0606, 0x0607, 0x0608, 0x0609, 
  0x0700, 0x0701, 0x0702, 0x0703, 0x0704, 0x0705, 0x0706, 0x0707, 0x0708, 0x0709, 
  0x0800, 0x0801, 0x0802, 0x0803, 0x0804, 0x0805, 0x0806, 0x0807, 0x0808, 0x0809, 
  0x0900, 0x0901, 0x0902, 0x0903, 0x0904, 0x0905, 0x0906, 0x0907, 0x0908, 0x0909, 
};

/* pre-build TOC */
static const uint16 toc_snatcher[21] =
{
  56014,   495, 10120, 20555, 1580, 5417, 12502, 16090,  6553, 9681,
   8148, 20228,  8622,  6142, 5858, 1287,  7424,  3535, 31697, 2485,
  31380
};

static const uint16 toc_lunar[52] =
{
  5422, 1057, 7932, 5401, 6380, 6592, 5862,  5937, 5478, 5870,
  6673, 6613, 6429, 4996, 4977, 5657, 3720,  5892, 3140, 3263,
  6351, 5187, 3249, 1464, 1596, 1750, 1751,  6599, 4578, 5205,
  1550, 1827, 2328, 1346, 1569, 1613, 7199,  4928, 1656, 2549,
  1875, 3901, 1850, 2399, 2028, 1724, 4889, 14551, 1184, 2132,
  685, 3167
};

static const uint32 toc_shadow[15] =
{
  10226, 70054, 11100, 12532, 12444, 11923, 10059, 10167, 10138, 13792,
  11637,  2547,  2521,  3856, 900
};

static const uint32 toc_dungeon[13] =
{
  2250, 22950, 16350, 24900, 13875, 19950, 13800, 15375, 17400, 17100,
  3325,  6825, 25275
};

static const uint32 toc_ffight[26] =
{
  11994, 9742, 10136, 9685, 9553, 14588, 9430, 8721, 9975, 9764,
  9704, 12796, 585, 754, 951, 624, 9047, 1068, 817, 9191, 1024,
  14562, 10320, 8627, 3795, 3047
};

static const uint32 toc_ffightj[29] =
{
  11994, 9752, 10119, 9690, 9567, 14575, 9431, 8731, 9965, 9763,
  9716, 12791, 579, 751, 958, 630, 9050, 1052, 825, 9193, 1026,
  14553, 9834, 10542, 1699, 1792, 1781, 3783, 3052
};

/* supported WAVE file header (16-bit stereo samples @44.1kHz) */
static const unsigned char waveHeader[32] =
{
  0x57,0x41,0x56,0x45,0x66,0x6d,0x74,0x20,0x10,0x00,0x00,0x00,0x01,0x00,0x02,0x00,
  0x44,0xac,0x00,0x00,0x10,0xb1,0x02,0x00,0x04,0x00,0x10,0x00,0x64,0x61,0x74,0x61
};

/* supported WAVE file extensions */
static const char extensions[SUPPORTED_EXT][16] =
{
#if defined(USE_LIBTREMOR) || defined(USE_LIBVORBIS)
  "%02d.ogg",
  " %02d.ogg",
  "-%02d.ogg",
  "_%02d.ogg",
  " - %02d.ogg",
  "%d.ogg",
  " %d.ogg",
  "-%d.ogg",
  "_%d.ogg",
  " - %d.ogg",
#endif
  "%02d.wav",
  " %02d.wav",
  "-%02d.wav",
  "_%02d.wav",
  " - %02d.wav",
  "%d.wav",
  " %d.wav",
  "-%d.wav",
  "_%d.wav",
  " - %d.wav"
};

#if defined(USE_LIBTREMOR) || defined(USE_LIBVORBIS)
#ifdef DISABLE_MANY_OGG_OPEN_FILES
static void ogg_free(int i)
{
  /* clear OGG file descriptor to prevent file from being closed */
  cdd.toc.tracks[i].vf.datasource = NULL;

  /* close VORBIS file structure */
  ov_clear(&cdd.toc.tracks[i].vf);

  /* indicates that the track is a seekable VORBIS file */
  cdd.toc.tracks[i].vf.seekable = 1;

  /* reset file reading position */
  fseek(cdd.toc.tracks[i].fd, 0, SEEK_SET);
}
#endif
#endif

void cdd_init(int samplerate)
{
  /* CD-DA is running by default at 44100 Hz */
  /* Audio stream is resampled to desired rate using Blip Buffer */
  blip_set_rates(snd.blips[2][0], 44100, samplerate);
  blip_set_rates(snd.blips[2][1], 44100, samplerate);
}

void cdd_reset(void)
{
  /* reset cycle counter */
  cdd.cycles = 0;
  
  /* reset drive access latency */
  cdd.latency = 0;
  
  /* reset track index */
  cdd.index = 0;
  
  /* reset logical block address */
  cdd.lba = 0;

  /* reset status */
  cdd.status = cdd.loaded ? CD_STOP : NO_DISC;
  
  /* reset CD-DA fader (full volume) */
  cdd.volume = 0x400;

  /* clear CD-DA output */
  cdd.audio[0] = cdd.audio[1] = 0;
}

int cdd_context_save(uint8 *state)
{
  int bufferptr = 0;

  save_param(&cdd.cycles, sizeof(cdd.cycles));
  save_param(&cdd.latency, sizeof(cdd.latency));
  save_param(&cdd.index, sizeof(cdd.index));
  save_param(&cdd.lba, sizeof(cdd.lba));
  save_param(&cdd.scanOffset, sizeof(cdd.scanOffset));
  save_param(&cdd.volume, sizeof(cdd.volume));
  save_param(&cdd.status, sizeof(cdd.status));

  return bufferptr;
}

int cdd_context_load(uint8 *state)
{
  int lba;
  int bufferptr = 0;

#if defined(USE_LIBTREMOR) || defined(USE_LIBVORBIS)
#ifdef DISABLE_MANY_OGG_OPEN_FILES
  /* close previous track VORBIS file structure to save memory */
  if (cdd.toc.tracks[cdd.index].vf.datasource)
  {
    ogg_free(cdd.index);
  }
#endif
#endif

  load_param(&cdd.cycles, sizeof(cdd.cycles));
  load_param(&cdd.latency, sizeof(cdd.latency));
  load_param(&cdd.index, sizeof(cdd.index));
  load_param(&cdd.lba, sizeof(cdd.lba));
  load_param(&cdd.scanOffset, sizeof(cdd.scanOffset));
  load_param(&cdd.volume, sizeof(cdd.volume));
  load_param(&cdd.status, sizeof(cdd.status));

  /* adjust current LBA within track limit */
  lba = cdd.lba;
  if (lba < cdd.toc.tracks[cdd.index].start)
  {
    lba = cdd.toc.tracks[cdd.index].start;
  }

  /* seek to current track position */
  if (!cdd.index)
  {
    /* DATA track */
    if (cdd.toc.tracks[0].fd)
    {
      fseek(cdd.toc.tracks[0].fd, lba * cdd.sectorSize, SEEK_SET);
    }
  }
#if defined(USE_LIBTREMOR) || defined(USE_LIBVORBIS)
  else if (cdd.toc.tracks[cdd.index].vf.seekable)
  {
#ifdef DISABLE_MANY_OGG_OPEN_FILES
    /* VORBIS file need to be opened first */
    ov_open(cdd.toc.tracks[cdd.index].fd,&cdd.toc.tracks[cdd.index].vf,0,0);
#endif
    /* VORBIS AUDIO track */
    ov_pcm_seek(&cdd.toc.tracks[cdd.index].vf, (lba - cdd.toc.tracks[cdd.index].start) * 588 - cdd.toc.tracks[cdd.index].offset);
  }
#endif
  else if (cdd.toc.tracks[cdd.index].fd)
  {
    /* PCM AUDIO track */
    fseek(cdd.toc.tracks[cdd.index].fd, (lba * 2352) - cdd.toc.tracks[cdd.index].offset, SEEK_SET);
  }

  return bufferptr;
}

int cdd_load(char *filename, char *header)
{
  char fname[256];
  char line[128];
  char *ptr = 0;
  char *lptr = 0;
  FILE *fd;

  /* first unmount any loaded disc */
  cdd_unload();

  /* open file */
  fd = fopen(filename, "rb");

  /* save a copy of base filename */
  strncpy(fname, filename, 256);

  /* autodetect .cue file */
  if (!memcmp(".cue", &filename[strlen(filename) - 4], 4) || !memcmp(".CUE", &filename[strlen(filename) - 4], 4))
  {
    if (fd)
    {
      /* find first FILE command */
      while (!lptr)
      {
        if (fgets(line, 128, fd) == NULL)
        {
          break;
        }
        lptr = strstr(line, "FILE");
      }

      /* get BINARY file name  */
      if (lptr && strstr(line, " BINARY"))
      {
        /* skip "FILE" attribute */
        lptr += 4;

        /* skip DOUBLE QUOTE or SPACE characters */
        while ((*lptr == 0x20) || (*lptr == '\"')) lptr++;

        /* set pointer at the end of filepath */
        ptr = fname + strlen(fname) - 1;
        while ((ptr - fname) && (*ptr != '/') && (*ptr != '\\')) ptr--;
        if (ptr - fname) ptr++;

        /* append filename characters after filepath */
        while ((*lptr != '\"') && memcmp(lptr, " BINARY", 7) && (ptr < (fname + 255)))
        {
          *ptr++ = *lptr++;
        }
        *ptr = 0;

        /* open file & initialize DATA track file descriptor */
        cdd.toc.tracks[0].fd = fopen(fname, "rb");
      }
      else
      {
        /* close .cue file */
        fclose(fd);

        /* invalid .cue file */
        return -1;
      }
    }
  }
  else
  {
    /* initialize DATA track file descriptor */
    cdd.toc.tracks[0].fd = fd;

    /* automatically try to open associated .cue file  */
    strncpy(&fname[strlen(fname) - 4], ".cue", 4);
    fd = fopen(fname, "rb");
  }

  if (!cdd.toc.tracks[0].fd)
  {
    /* close any opened .cue file */
    if (fd) fclose(fd);

    /* error opening file */
    return -1;
  }

  /* read first 16 bytes */
  fread(header, 0x10, 1, cdd.toc.tracks[0].fd);

  /* look for valid CD image ID string */
  if (memcmp("SEGADISCSYSTEM", header, 14))
  {    
    /* if not found, read next 16 bytes */
    fread(header, 0x10, 1, cdd.toc.tracks[0].fd);

    /* look again for valid CD image ID string */
    if (memcmp("SEGADISCSYSTEM", header, 14))
    {
      /* close any opened .cue file */
      if (fd) fclose(fd);

      /* close binary file */
      fclose(cdd.toc.tracks[0].fd);
      cdd.toc.tracks[0].fd = 0;

      /* not a CD image file */
      return 0;
    }

    /* BIN format (2352 bytes data blocks) */
    cdd.sectorSize = 2352;
  }
  else
  {
    /* ISO format (2048 bytes data blocks) */
    cdd.sectorSize = 2048;
  }

  /* read CD image header + security code */
  fread(header + 0x10, 0x200, 1, cdd.toc.tracks[0].fd);

  /* DATA track start time (based on DATA file length) */
  fseek(cdd.toc.tracks[0].fd, 0, SEEK_END);
  cdd.toc.tracks[0].end = ftell(cdd.toc.tracks[0].fd) / cdd.sectorSize;
  
  /* DATA track start time (logical block 0) */
  fseek(cdd.toc.tracks[0].fd, 0, SEEK_SET);
  cdd.toc.tracks[0].start = 0;

  /* initialize TOC */
  cdd.toc.end = cdd.toc.tracks[0].end;
  cdd.toc.last = 1;

  /* automatically retrieve audio tracks infos from .cue file */
  if (fd)
  {
    int pregap = 0;
    int mm, ss, bb;
        
    /* skip first (DATA) track */
    while (!strstr(line, "INDEX 01") && !strstr(line, "INDEX 1"))
    {
      if (fgets(line, 128, fd) == NULL)
      {
        break;
      }
    }

    /* read next lines until end of file */
    while (fgets(line, 128, fd) != NULL)
    {
      /* skip any SPACE characters */
      lptr = line;
      while (*lptr == 0x20) lptr++;

      /* decode FILE commands */
      if (!(memcmp(lptr, "FILE", 4)))
      {
        /* skip "FILE" attribute */
        lptr += 4;

        /* skip DOUBLE QUOTE or SPACE characters */
        while ((*lptr == 0x20) || (*lptr == '\"')) lptr++;

        /* set pointer at the end of filepath */
        ptr = fname + strlen(fname) - 1;
        while ((ptr - fname) && (*ptr != '/') && (*ptr != '\\')) ptr--;
        if (ptr - fname) ptr++;

        /* append filename characters after filepath */
        while ((*lptr != '\"') && (lptr <= (line + 128)) && (ptr < (fname + 255)))
        {
          *ptr++ = *lptr++;
        }
        *ptr = 0;

        /* open file & initialize track file descriptor */
        cdd.toc.tracks[cdd.toc.last].fd = fopen(fname, "rb");
        if (!cdd.toc.tracks[cdd.toc.last].fd)
        {
          /* error opening file */
          break;
        }

        /* reset current file PREGAP length */
        pregap = 0;

        /* reset current file read offset */
        cdd.toc.tracks[cdd.toc.last].offset = 0;

        /* check supported file types */
        if (!strstr(lptr,"BINARY"))
        {
          /* read file header */
          unsigned char head[32];
          fseek(cdd.toc.tracks[cdd.toc.last].fd, 8, SEEK_SET);
          fread(head, 32, 1, cdd.toc.tracks[cdd.toc.last].fd);
          fseek(cdd.toc.tracks[cdd.toc.last].fd, 0, SEEK_SET);
      
          /* autodetect WAVE file header (16-bit stereo @44.1kHz only) */
          if (!memcmp(head, waveHeader, 32))
          {
            /* adjust current track file read offset with WAVE header length */
            cdd.toc.tracks[cdd.toc.last].offset -= 44;
          }
#if defined(USE_LIBTREMOR) || defined(USE_LIBVORBIS)
          else if (!ov_open(cdd.toc.tracks[cdd.toc.last].fd,&cdd.toc.tracks[cdd.toc.last].vf,0,0))
          {
            /* retrieve stream infos */
            vorbis_info *info = ov_info(&cdd.toc.tracks[cdd.toc.last].vf,-1);
            if (!info || (info->rate != 44100) || (info->channels != 2))
            {
              /* unsupported VORBIS file format (stereo @44.1kHz only) */
              ov_clear(&cdd.toc.tracks[cdd.toc.last].vf);
              cdd.toc.tracks[cdd.toc.last].fd = 0;
              break;
            }
          }
#endif
          else
          {
            /* unsupported audio file */
            fclose(cdd.toc.tracks[cdd.toc.last].fd);
            cdd.toc.tracks[cdd.toc.last].fd = 0;
            break;
          }
        }
      }

      /* decode TRACK commands */
      else if ((sscanf(lptr, "TRACK %02d AUDIO", &bb)) || (sscanf(lptr, "TRACK %d AUDIO", &bb)))
      {
        /* check track number */
        if (bb != (cdd.toc.last + 1))
        {
          /* close any opened file */
          if (cdd.toc.tracks[cdd.toc.last].fd)
          {
            fclose(cdd.toc.tracks[cdd.toc.last].fd);
            cdd.toc.tracks[cdd.toc.last].fd = 0;
          }

          /* missing tracks */
          break;
        }

        /* check if a single file is used for all tracks */
        if (!cdd.toc.tracks[cdd.toc.last].fd)
        {
          /* clear previous track end time */
          cdd.toc.tracks[cdd.toc.last - 1].end = 0;
        }
      }

      /* decode PREGAP commands */
      else if (sscanf(lptr, "PREGAP %02d:%02d:%02d", &mm, &ss, &bb) == 3)
      {
        /* increment current file PREGAP length */
        pregap += bb + ss*75 + mm*60*75;
      }

      /* decode INDEX commands */
      else if ((sscanf(lptr, "INDEX 00 %02d:%02d:%02d", &mm, &ss, &bb) == 3) ||
                (sscanf(lptr, "INDEX 0 %02d:%02d:%02d", &mm, &ss, &bb) == 3))
      {
        /* check if a single file is used for all tracks */
        if (!cdd.toc.tracks[cdd.toc.last].fd)
        {
          /* set previous track end time */
          cdd.toc.tracks[cdd.toc.last - 1].end = bb + ss*75 + mm*60*75 + pregap;
        }
      }
      else if ((sscanf(lptr, "INDEX 01 %02d:%02d:%02d", &mm, &ss, &bb) == 3) ||
                (sscanf(lptr, "INDEX 1 %02d:%02d:%02d", &mm, &ss, &bb) == 3))
      {
        /* adjust file read offset for current track with current file PREGAP length */
        cdd.toc.tracks[cdd.toc.last].offset += pregap * 2352;

        /* check if a single file is used for all tracks */
        if (!cdd.toc.tracks[cdd.toc.last].fd)
        {
          /* use common file descriptor */
          cdd.toc.tracks[cdd.toc.last].fd = cdd.toc.tracks[0].fd;

          /* current track start time (based on current absolute time) */
          cdd.toc.tracks[cdd.toc.last].start = bb + ss*75 + mm*60*75 + pregap;

          /* check if previous track end time has not been already set (through INDEX00 command) */
          if (cdd.toc.tracks[cdd.toc.last - 1].end == 0)
          {
            /* set previous track end time (based on current absolute timee) */
            cdd.toc.tracks[cdd.toc.last - 1].end = bb + ss*75 + mm*60*75;
          }
        }
        else
        {
          /* current track start time (based on previous track end time) */
          cdd.toc.tracks[cdd.toc.last].start = cdd.toc.end + bb + ss*75 + mm*60*75 + pregap;

          /* adjust file read offset with previous track end time */
          cdd.toc.tracks[cdd.toc.last].offset += cdd.toc.end * 2352;

#if defined(USE_LIBTREMOR) || defined(USE_LIBVORBIS)
          if (cdd.toc.tracks[cdd.toc.last].vf.datasource)
          { 
            /* convert read offset to PCM sample offset */
            cdd.toc.tracks[cdd.toc.last].offset = cdd.toc.tracks[cdd.toc.last].offset / 4;

            /* current track end time */
            cdd.toc.tracks[cdd.toc.last].end = cdd.toc.tracks[cdd.toc.last].start + ov_pcm_total(&cdd.toc.tracks[cdd.toc.last].vf,-1)/588;
            if (cdd.toc.tracks[cdd.toc.last].end <= cdd.toc.tracks[cdd.toc.last].start)
            {
              /* invalid length */
              ov_clear(&cdd.toc.tracks[cdd.toc.last].vf);
              cdd.toc.tracks[cdd.toc.last].fd = 0;
              cdd.toc.tracks[cdd.toc.last].end = 0;
              cdd.toc.tracks[cdd.toc.last].start = 0;
              cdd.toc.tracks[cdd.toc.last].offset = 0;
              break;
            }

#ifdef DISABLE_MANY_OGG_OPEN_FILES
            /* close VORBIS file structure to save memory */
            ogg_free(cdd.toc.last);
#endif
          } 
          else
#endif
          {
            /* current track end time */
            fseek(cdd.toc.tracks[cdd.toc.last].fd, 0, SEEK_END);
            cdd.toc.tracks[cdd.toc.last].end = cdd.toc.tracks[cdd.toc.last].start + ((ftell(cdd.toc.tracks[cdd.toc.last].fd) + 2351) / 2352);
            fseek(cdd.toc.tracks[cdd.toc.last].fd, 0, SEEK_SET);
          }

          /* update TOC end */
          cdd.toc.end = cdd.toc.tracks[cdd.toc.last].end;
        }
     
        /* increment track number */
        cdd.toc.last++;
      }
    }

    /* check if a single file is used for all tracks */
    if (cdd.toc.tracks[cdd.toc.last - 1].fd == cdd.toc.tracks[0].fd)
    {
      /* adjust TOC end */
      cdd.toc.end += pregap;

      /* last track end index */
      cdd.toc.tracks[cdd.toc.last - 1].end = cdd.toc.end;
    }

    /* close .cue file */
    fclose(fd);
  }

  /* .ISO audio tracks auto-detection */
  else if (cdd.sectorSize == 2048)
  {
    int i, offset;

    /* set pointer at the end of filename */
    ptr = fname + strlen(fname) - 4;

    /* auto-detect track file extensions */
    for (i=0; i<SUPPORTED_EXT; i++)
    {
      /* auto-detect bad rips with wrong track indexes */
      sprintf(ptr, extensions[i], 1);
      fd = fopen(fname, "rb");
      if (fd)
      {
        offset = 0;
        break;
      }

      sprintf(ptr, extensions[i], 2);
      fd = fopen(fname, "rb");
      if (fd)
      {
        offset = 1;
        break;
      }
    }

    /* repeat until no more valid track files can be found */
    while (fd)
    {
      /* read file HEADER */
      unsigned char head[32];
      fseek(fd, 8, SEEK_SET);
      fread(head, 32, 1, fd);
      fseek(fd, 0, SEEK_SET);
      
      /* check if this is a valid WAVE file (16-bit stereo @44.1kHz only) */
      if (!memcmp(head, waveHeader, 32))
      {
        /* initialize current track file descriptor */
        cdd.toc.tracks[cdd.toc.last].fd = fd;

        /* initialize current track start time (based on previous track end time) */
        cdd.toc.tracks[cdd.toc.last].start = cdd.toc.end;

        /* add default 2s PAUSE between tracks */
        cdd.toc.tracks[cdd.toc.last].start += 150;

        /* current track end time */
        fseek(fd, 0, SEEK_END);
        cdd.toc.tracks[cdd.toc.last].end = cdd.toc.tracks[cdd.toc.last].start + ((ftell(fd) + 2351) / 2352);

        /* initialize file read offset for current track */
        cdd.toc.tracks[cdd.toc.last].offset = cdd.toc.tracks[cdd.toc.last].start * 2352;

        /* auto-detect PAUSE within audio files */
        fseek(fd, 100 * 2352, SEEK_SET);
        fread(head, 4, 1, fd);
        fseek(fd, 0, SEEK_SET);
        if (*(int32 *)head == 0)
        {
          /* assume 2s PAUSE is included at the beginning of the file */
          cdd.toc.tracks[cdd.toc.last].offset -= 150 * 2352;
          cdd.toc.tracks[cdd.toc.last].end -= 150;
        }

        /* update TOC end */
        cdd.toc.end = cdd.toc.tracks[cdd.toc.last].end;

        /* adjust file read offset for current track with WAVE header length */
        cdd.toc.tracks[cdd.toc.last].offset -= 44;

        /* increment track number */
        cdd.toc.last++;
      }
#if defined(USE_LIBTREMOR) || defined(USE_LIBVORBIS)
      else if (!ov_open(fd,&cdd.toc.tracks[cdd.toc.last].vf,0,0))
      {
        /* retrieve stream infos */
        vorbis_info *info = ov_info(&cdd.toc.tracks[cdd.toc.last].vf,-1);
        if (!info || (info->rate != 44100) || (info->channels != 2))
        {
          /* unsupported OGG file */
          ov_clear(&cdd.toc.tracks[cdd.toc.last].vf);
          break;
        }

        /* initialize current track file descriptor */
        cdd.toc.tracks[cdd.toc.last].fd = fd;

        /* initialize current track start time (based on previous track end time) */
        cdd.toc.tracks[cdd.toc.last].start = cdd.toc.end;

        /* add default 2s PAUSE between tracks */
        cdd.toc.tracks[cdd.toc.last].start += 150;

        /* current track end time */
        cdd.toc.tracks[cdd.toc.last].end = cdd.toc.tracks[cdd.toc.last].start + ov_pcm_total(&cdd.toc.tracks[cdd.toc.last].vf,-1)/588;
        if (cdd.toc.tracks[cdd.toc.last].end <= cdd.toc.tracks[cdd.toc.last].start)
        {
          /* invalid file length */
          ov_clear(&cdd.toc.tracks[cdd.toc.last].vf);
          cdd.toc.tracks[cdd.toc.last].fd = 0;
          cdd.toc.tracks[cdd.toc.last].end = 0;
          cdd.toc.tracks[cdd.toc.last].start = 0;
          break;
        }

        /* auto-detect PAUSE within audio files */
        ov_pcm_seek(&cdd.toc.tracks[cdd.toc.last].vf, 100 * 588);
#if defined(USE_LIBVORBIS)
        ov_read(&cdd.toc.tracks[cdd.toc.last].vf, (char *)head, 32, 0, 2, 1, 0);
#else
        ov_read(&cdd.toc.tracks[cdd.toc.last].vf, (char *)head, 32, 0);
#endif
        ov_pcm_seek(&cdd.toc.tracks[cdd.toc.last].vf, 0);
        if (*(int32 *)head == 0)
        {
          /* assume 2s PAUSE is included at the beginning of the file */
          cdd.toc.tracks[cdd.toc.last].offset -= 150 * 588;
          cdd.toc.tracks[cdd.toc.last].end -= 150;
        }

#ifdef DISABLE_MANY_OGG_OPEN_FILES
        /* close VORBIS file structure to save memory */
        ogg_free(cdd.toc.last);
#endif

        /* update TOC end */
        cdd.toc.end = cdd.toc.tracks[cdd.toc.last].end;

        /* increment track number */
        cdd.toc.last++;
      }
#endif
      else
      {
        /* unsupported audio file format */
        fclose(fd);
        break;
      }

      /* try to open next audio track file */
      sprintf(ptr, extensions[i], cdd.toc.last + offset);
      fd = fopen(fname, "rb");
    }
  }

  /* Simulate audio tracks if none found */
  if (cdd.toc.last == 1)
  {
    /* Some games require exact TOC infos */
    if (strstr(header + 0x180,"T-95035") != NULL)
    {
      /* Snatcher */
      cdd.toc.last = cdd.toc.end = 0;
      do
      {
        cdd.toc.tracks[cdd.toc.last].start = cdd.toc.end;
        cdd.toc.tracks[cdd.toc.last].end = cdd.toc.tracks[cdd.toc.last].start + toc_snatcher[cdd.toc.last];
        cdd.toc.end = cdd.toc.tracks[cdd.toc.last].end;
        cdd.toc.last++;
      }
      while (cdd.toc.last < 21);
    }
    else if (strstr(header + 0x180,"T-127015") != NULL)
    {
      /* Lunar - The Silver Star */
      cdd.toc.last = cdd.toc.end = 0;
      do
      {
        cdd.toc.tracks[cdd.toc.last].start = cdd.toc.end;
        cdd.toc.tracks[cdd.toc.last].end = cdd.toc.tracks[cdd.toc.last].start + toc_lunar[cdd.toc.last];
        cdd.toc.end = cdd.toc.tracks[cdd.toc.last].end;
        cdd.toc.last++;
      }
      while (cdd.toc.last < 52);
    }
    else if (strstr(header + 0x180,"T-113045") != NULL)
    {
      /* Shadow of the Beast II */
      cdd.toc.last = cdd.toc.end = 0;
      do
      {
        cdd.toc.tracks[cdd.toc.last].start = cdd.toc.end;
        cdd.toc.tracks[cdd.toc.last].end = cdd.toc.tracks[cdd.toc.last].start + toc_shadow[cdd.toc.last];
        cdd.toc.end = cdd.toc.tracks[cdd.toc.last].end;
        cdd.toc.last++;
      }
      while (cdd.toc.last < 15);
    }
    else if (strstr(header + 0x180,"T-143025") != NULL)
    {
      /* Dungeon Explorer */
      cdd.toc.last = cdd.toc.end = 0;
      do
      {
        cdd.toc.tracks[cdd.toc.last].start = cdd.toc.end;
        cdd.toc.tracks[cdd.toc.last].end = cdd.toc.tracks[cdd.toc.last].start + toc_dungeon[cdd.toc.last];
        cdd.toc.end = cdd.toc.tracks[cdd.toc.last].end;
        cdd.toc.last++;
      }
      while (cdd.toc.last < 13);
    }
    else if (strstr(header + 0x180,"MK-4410") != NULL)
    {
      /* Final Fight CD (USA, Europe) */
      cdd.toc.last = cdd.toc.end = 0;
      do
      {
        cdd.toc.tracks[cdd.toc.last].start = cdd.toc.end;
        cdd.toc.tracks[cdd.toc.last].end = cdd.toc.tracks[cdd.toc.last].start + toc_ffight[cdd.toc.last];
        cdd.toc.end = cdd.toc.tracks[cdd.toc.last].end;
        cdd.toc.last++;
      }
      while (cdd.toc.last < 26);
    }
    else if (strstr(header + 0x180,"G-6013") != NULL)
    {
      /* Final Fight CD (Japan) */
      cdd.toc.last = cdd.toc.end = 0;
      do
      {
        cdd.toc.tracks[cdd.toc.last].start = cdd.toc.end;
        cdd.toc.tracks[cdd.toc.last].end = cdd.toc.tracks[cdd.toc.last].start + toc_ffightj[cdd.toc.last];
        cdd.toc.end = cdd.toc.tracks[cdd.toc.last].end;
        cdd.toc.last++;
      }
      while (cdd.toc.last < 29);
    }
    else
    {
      /* default TOC (99 tracks & 2s per audio tracks) */
      do
      {
        cdd.toc.tracks[cdd.toc.last].start = cdd.toc.end + 2*75;
        cdd.toc.tracks[cdd.toc.last].end = cdd.toc.tracks[cdd.toc.last].start + 2*75;
        cdd.toc.end = cdd.toc.tracks[cdd.toc.last].end;
        cdd.toc.last++;
      }
      while ((cdd.toc.last < 99) && (cdd.toc.end < 56*60*75));
    }
  }

  /* Lead-out */
  cdd.toc.tracks[cdd.toc.last].start = cdd.toc.end;

  /* CD loaded */
  cdd.loaded = 1;
  return 1;
}

void cdd_unload(void)
{
  if (cdd.loaded)
  {
    int i;

    /* close CD tracks */
    for (i=0; i<cdd.toc.last; i++)
    {
#if defined(USE_LIBTREMOR) || defined(USE_LIBVORBIS)
      if (cdd.toc.tracks[i].vf.datasource)
      {
        /* close VORBIS file (if still opened) */
        ov_clear(&cdd.toc.tracks[i].vf);
      }
      else
#endif
      if (cdd.toc.tracks[i].fd)
      {
        /* close file */
        fclose(cdd.toc.tracks[i].fd);

        /* detect single file images */
        if (cdd.toc.tracks[i+1].fd == cdd.toc.tracks[i].fd)
        {
          /* exit loop */
          i = cdd.toc.last;
        }
      }
    }

    /* CD unloaded */
    cdd.loaded = 0;
  }

  /* reset TOC */
  memset(&cdd.toc, 0x00, sizeof(cdd.toc));
    
  /* unknown CD image file format */
  cdd.sectorSize = 0;
}

void cdd_read_data(uint8 *dst)
{
  /* only read DATA track sectors */
  if ((cdd.lba >= 0) && (cdd.lba < cdd.toc.tracks[0].end))
  {
    /* BIN format ? */
    if (cdd.sectorSize == 2352)
    {
      /* skip 16-byte header */
      fseek(cdd.toc.tracks[0].fd, cdd.lba * 2352 + 16, SEEK_SET);
    }

    /* read sector data (Mode 1 = 2048 bytes) */
    fread(dst, 2048, 1, cdd.toc.tracks[0].fd);
  }
}

void cdd_read_audio(unsigned int samples)
{
  /* previous audio outputs */
  int16 l = cdd.audio[0];
  int16 r = cdd.audio[1];

  /* get number of internal clocks (samples) needed */
  samples = blip_clocks_needed(snd.blips[2][0], samples);

  /* audio track playing ? */
  if (!scd.regs[0x36>>1].byte.h && cdd.toc.tracks[cdd.index].fd)
  {
    int i, mul, delta;

    /* current CD-DA fader volume */
    int curVol = cdd.volume;

    /* CD-DA fader volume setup (0-1024) */
    int endVol = scd.regs[0x34>>1].w >> 4;

    /* read samples from current block */
#if defined(USE_LIBTREMOR) || defined(USE_LIBVORBIS)
    if (cdd.toc.tracks[cdd.index].vf.datasource)
    {
      int len, done = 0;
      int16 *ptr = (int16 *) (cdc.ram);
      samples = samples * 4;
      while (done < samples)
      {
#ifdef USE_LIBVORBIS
        len = ov_read(&cdd.toc.tracks[cdd.index].vf, (char *)(cdc.ram + done), samples - done, 0, 2, 1, 0);
#else
        len = ov_read(&cdd.toc.tracks[cdd.index].vf, (char *)(cdc.ram + done), samples - done, 0);
#endif
        if (len <= 0) 
        {
          done = samples;
          break;
        }
        done += len;
      }
      samples = done / 4;

      /* process 16-bit (host-endian) stereo samples */
      for (i=0; i<samples; i++)
      {
        /* CD-DA fader multiplier (cf. LC7883 datasheet) */
        /* (MIN) 0,1,2,3,4,8,12,16,20...,1020,1024 (MAX) */
        mul = (curVol & 0x7fc) ? (curVol & 0x7fc) : (curVol & 0x03);

        /* left channel */
        delta = ((ptr[0] * mul) / 1024) - l;
        ptr++;
        l += delta;
        blip_add_delta_fast(snd.blips[2][0], i, delta);

        /* right channel */
        delta = ((ptr[0] * mul) / 1024) - r;
        ptr++;
        r += delta;
        blip_add_delta_fast(snd.blips[2][1], i, delta);

        /* update CD-DA fader volume (one step/sample) */
        if (curVol < endVol)
        {
          /* fade-in */
          curVol++;
        }
        else if (curVol > endVol)
        {
          /* fade-out */
          curVol--;
        }
        else if (!curVol)
        {
          /* audio will remain muted until next setup */
          break;
        }
      }
    }
    else
#endif
    {
#ifdef LSB_FIRST
      int16 *ptr = (int16 *) (cdc.ram);
#else
      uint8 *ptr = cdc.ram;
#endif
      fread(cdc.ram, 1, samples * 4, cdd.toc.tracks[cdd.index].fd);

      /* process 16-bit (little-endian) stereo samples */
      for (i=0; i<samples; i++)
      {
        /* CD-DA fader multiplier (cf. LC7883 datasheet) */
        /* (MIN) 0,1,2,3,4,8,12,16,20...,1020,1024 (MAX) */
        mul = (curVol & 0x7fc) ? (curVol & 0x7fc) : (curVol & 0x03);

        /* left channel */
#ifdef LSB_FIRST
        delta = ((ptr[0] * mul) / 1024) - l;
        ptr++;
#else
        delta = (((int16)((ptr[0] + ptr[1]*256)) * mul) / 1024) - l;
        ptr += 2;
#endif
        l += delta;
        blip_add_delta_fast(snd.blips[2][0], i, delta);

        /* right channel */
#ifdef LSB_FIRST
        delta = ((ptr[0] * mul) / 1024) - r;
        ptr++;
#else
        delta = (((int16)((ptr[0] + ptr[1]*256)) * mul) / 1024) - r;
        ptr += 2;
#endif
        r += delta;
        blip_add_delta_fast(snd.blips[2][1], i, delta);

        /* update CD-DA fader volume (one step/sample) */
        if (curVol < endVol)
        {
          /* fade-in */
          curVol++;
        }
        else if (curVol > endVol)
        {
          /* fade-out */
          curVol--;
        }
        else if (!curVol)
        {
          /* audio will remain muted until next setup */
          break;
        }
      }
    }

    /* save current CD-DA fader volume */
    cdd.volume = curVol;

    /* save last audio output for next frame */
    cdd.audio[0] = l;
    cdd.audio[1] = r;
  }
  else
  {
    /* no audio output */
    if (l) blip_add_delta_fast(snd.blips[2][0], 0, -l);
    if (r) blip_add_delta_fast(snd.blips[2][1], 0, -r);

    /* save audio output for next frame */
    cdd.audio[0] = 0;
    cdd.audio[1] = 0;
  }

  /* end of Blip Buffer timeframe */
  blip_end_frame(snd.blips[2][0], samples);
  blip_end_frame(snd.blips[2][1], samples);
}


void cdd_update(void)
{  
#ifdef LOG_CDD
  error("LBA = %d (track n�%d)(latency=%d)\n", cdd.lba, cdd.index, cdd.latency);
#endif
  
  /* seeking disc */
  if (cdd.status == CD_SEEK)
  {
    /* drive latency */
    if (cdd.latency > 0)
    {
      cdd.latency--;
      return;
    }

    /* drive is ready */
    cdd.status = CD_READY;
  }

  /* reading disc */
  else if (cdd.status == CD_PLAY)
  {
    /* drive latency */
    if (cdd.latency > 0)
    {
      cdd.latency--;
      return;
    }

    /* track type */
    if (!cdd.index)
    {
      /* DATA sector header (CD-ROM Mode 1) */
      uint8 header[4];
      uint32 msf = cdd.lba + 150;
      header[0] = lut_BCD_8[(msf / 75) / 60];
      header[1] = lut_BCD_8[(msf / 75) % 60];
      header[2] = lut_BCD_8[(msf % 75)];
      header[3] = 0x01;

      /* data track sector read is controlled by CDC */
      cdd.lba += cdc_decoder_update(*(uint32 *)(header));
    }
    else if (cdd.index < cdd.toc.last)
    {
      /* check against audio track start index */
      if (cdd.lba >= cdd.toc.tracks[cdd.index].start)
      {
        /* audio track playing */
        scd.regs[0x36>>1].byte.h = 0x00;
      }

      /* audio blocks are still sent to CDC as well as CD DAC/Fader */
      cdc_decoder_update(0);
 
      /* next audio block is automatically read */
      cdd.lba++;
    }
    else
    {
      /* end of disc */
      cdd.status = CD_END;
      return;
    }

    /* check end of current track */
    if (cdd.lba >= cdd.toc.tracks[cdd.index].end)
    {
#if defined(USE_LIBTREMOR) || defined(USE_LIBVORBIS)
#ifdef DISABLE_MANY_OGG_OPEN_FILES
      /* close previous track VORBIS file structure to save memory */
      if (cdd.toc.tracks[cdd.index].vf.datasource)
      {
        ogg_free(cdd.index);
      }
#endif
#endif
      /* play next track */
      cdd.index++;

      /* PAUSE between tracks */
      scd.regs[0x36>>1].byte.h = 0x01;

      /* seek to next audio track start */
#if defined(USE_LIBTREMOR) || defined(USE_LIBVORBIS)
      if (cdd.toc.tracks[cdd.index].vf.seekable)
      {
#ifdef DISABLE_MANY_OGG_OPEN_FILES
        /* VORBIS file need to be opened first */
        ov_open(cdd.toc.tracks[cdd.index].fd,&cdd.toc.tracks[cdd.index].vf,0,0);
#endif
        ov_pcm_seek(&cdd.toc.tracks[cdd.index].vf, -cdd.toc.tracks[cdd.index].offset);
      }
      else
#endif 
      if (cdd.toc.tracks[cdd.index].fd)
      {
        fseek(cdd.toc.tracks[cdd.index].fd, (cdd.toc.tracks[cdd.index].start * 2352) - cdd.toc.tracks[cdd.index].offset, SEEK_SET);
      }
    }
  }

  /* scanning disc */
  else if (cdd.status == CD_SCAN)
  {
    /* fast-forward or fast-rewind */
    cdd.lba += cdd.scanOffset;

    /* check current track limits */
    if (cdd.lba >= cdd.toc.tracks[cdd.index].end)
    {
#if defined(USE_LIBTREMOR) || defined(USE_LIBVORBIS)
#ifdef DISABLE_MANY_OGG_OPEN_FILES
      /* close previous track VORBIS file structure to save memory */
      if (cdd.toc.tracks[cdd.index].vf.datasource)
      {
        ogg_free(cdd.index);
      }
#endif
#endif
      /* next track */
      cdd.index++;

      /* skip directly to track start position */
      cdd.lba = cdd.toc.tracks[cdd.index].start;
      
      /* AUDIO track playing ? */
      if (cdd.status == CD_PLAY)
      {
      	scd.regs[0x36>>1].byte.h = 0x00;
      }
    }
    else if (cdd.lba < cdd.toc.tracks[cdd.index].start)
    {
#if defined(USE_LIBTREMOR) || defined(USE_LIBVORBIS)
#ifdef DISABLE_MANY_OGG_OPEN_FILES
      /* close previous track VORBIS file structure to save memory */
      if (cdd.toc.tracks[cdd.index].vf.datasource)
      {
        ogg_free(cdd.index);
      }
#endif
#endif

      /* previous track */
      cdd.index--;

      /* skip directly to track end position */
      cdd.lba = cdd.toc.tracks[cdd.index].end;
    }

    /* check disc limits */
    if (cdd.index < 0)
    {
      cdd.index = 0;
      cdd.lba = 0;
    }
    else if (cdd.index >= cdd.toc.last)
    {
      /* no AUDIO track playing */
      scd.regs[0x36>>1].byte.h = 0x01;

      /* end of disc */
      cdd.index = cdd.toc.last;
      cdd.lba = cdd.toc.end;
      cdd.status = CD_END;
      return;
    }

    /* seek to current block */
    if (!cdd.index)
    {
      /* no AUDIO track playing */
      scd.regs[0x36>>1].byte.h = 0x01;

      /* DATA track */
      fseek(cdd.toc.tracks[0].fd, cdd.lba * cdd.sectorSize, SEEK_SET);
    }
#if defined(USE_LIBTREMOR) || defined(USE_LIBVORBIS)
    else if (cdd.toc.tracks[cdd.index].vf.seekable)
    {
#ifdef DISABLE_MANY_OGG_OPEN_FILES
      /* check if a new track is being played */
      if (!cdd.toc.tracks[cdd.index].vf.datasource)
      {
        /* VORBIS file need to be opened first */
        ov_open(cdd.toc.tracks[cdd.index].fd,&cdd.toc.tracks[cdd.index].vf,0,0);
      }
#endif
      /* VORBIS AUDIO track */
      ov_pcm_seek(&cdd.toc.tracks[cdd.index].vf, (cdd.lba - cdd.toc.tracks[cdd.index].start) * 588 - cdd.toc.tracks[cdd.index].offset);
    }
#endif 
    else if (cdd.toc.tracks[cdd.index].fd)
    {
      /* PCM AUDIO track */
      fseek(cdd.toc.tracks[cdd.index].fd, (cdd.lba * 2352) - cdd.toc.tracks[cdd.index].offset, SEEK_SET);
    }
  }
}

void cdd_process(void)
{
  /* Process CDD command */
  switch (scd.regs[0x42>>1].byte.h & 0x0f)
  {
    case 0x00:  /* Drive Status */
    {
      /* RS1-RS8 normally unchanged */
      scd.regs[0x38>>1].byte.h = cdd.status;

      /* unless RS1 indicated invalid track infos */
      if (scd.regs[0x38>>1].byte.l == 0x0f)
      {
        /* and SEEK has ended */
        if (cdd.status != CD_SEEK)
        {
          /* then return valid track infos, e.g current track number in RS2-RS3 (fixes Lunar - The Silver Star) */
          scd.regs[0x38>>1].byte.l = 0x02;
          scd.regs[0x3a>>1].w = (cdd.index < cdd.toc.last) ? lut_BCD_16[cdd.index + 1] : 0x0A0A;
        }
      }
      break;
    }

    case 0x01:  /* Stop Drive */
    {
      /* update status */
      cdd.status = cdd.loaded ? CD_STOP : NO_DISC;

      /* no audio track playing */
      scd.regs[0x36>>1].byte.h = 0x01;

      /* RS1-RS8 ignored, expects 0x0 ("no disc" ?) in RS0 once */
      scd.regs[0x38>>1].w = 0x0000;
      scd.regs[0x3a>>1].w = 0x0000;
      scd.regs[0x3c>>1].w = 0x0000;
      scd.regs[0x3e>>1].w = 0x0000;
      scd.regs[0x40>>1].w = 0x000f;
      return;
    }

    case 0x02:  /* Read TOC */
    {
      /* Infos automatically retrieved by CDD processor from Q-Channel */
      /* commands 0x00-0x02 (current block) and 0x03-0x05 (Lead-In) */
      switch (scd.regs[0x44>>1].byte.l)
      {
        case 0x00:  /* Current Absolute Time (MM:SS:FF) */
        {
          int lba = cdd.lba + 150;
          scd.regs[0x38>>1].w = cdd.status << 8;
          scd.regs[0x3a>>1].w = lut_BCD_16[(lba/75)/60];
          scd.regs[0x3c>>1].w = lut_BCD_16[(lba/75)%60];
          scd.regs[0x3e>>1].w = lut_BCD_16[(lba%75)];
          scd.regs[0x40>>1].byte.h = cdd.index ? 0x00 : 0x04; /* Current block flags in RS8 (bit0 = mute status, bit1: pre-emphasis status, bit2: track type) */
          break;
        }

        case 0x01:  /* Current Track Relative Time (MM:SS:FF) */
        {
          int lba = cdd.lba - cdd.toc.tracks[cdd.index].start;
          scd.regs[0x38>>1].w = (cdd.status << 8) | 0x01;
          scd.regs[0x3a>>1].w = lut_BCD_16[(lba/75)/60];
          scd.regs[0x3c>>1].w = lut_BCD_16[(lba/75)%60];
          scd.regs[0x3e>>1].w = lut_BCD_16[(lba%75)];
          scd.regs[0x40>>1].byte.h = cdd.index ? 0x00 : 0x04; /* Current block flags in RS8 (bit0 = mute status, bit1: pre-emphasis status, bit2: track type) */
          break;
        }

        case 0x02:  /* Current Track Number */
        {
          scd.regs[0x38>>1].w = (cdd.status << 8) | 0x02;
          scd.regs[0x3a>>1].w = (cdd.index < cdd.toc.last) ? lut_BCD_16[cdd.index + 1] : 0x0A0A;
          scd.regs[0x3c>>1].w = 0x0000;
          scd.regs[0x3e>>1].w = 0x0000; /* Disk Control Code (?) in RS6 */
          scd.regs[0x40>>1].byte.h = 0x00;
          break;
        }

        case 0x03:  /* Total length (MM:SS:FF) */
        {
          int lba = cdd.toc.end + 150;
          scd.regs[0x38>>1].w = (cdd.status << 8) | 0x03;
          scd.regs[0x3a>>1].w = lut_BCD_16[(lba/75)/60];
          scd.regs[0x3c>>1].w = lut_BCD_16[(lba/75)%60];
          scd.regs[0x3e>>1].w = lut_BCD_16[(lba%75)];
          scd.regs[0x40>>1].byte.h = 0x00;
          break;
        }

        case 0x04:  /* First & Last Track Numbers */
        {
          scd.regs[0x38>>1].w = (cdd.status << 8) | 0x04;
          scd.regs[0x3a>>1].w = 0x0001;
          scd.regs[0x3c>>1].w = lut_BCD_16[cdd.toc.last];
          scd.regs[0x3e>>1].w = 0x0000; /* Drive Version (?) in RS6-RS7 */
          scd.regs[0x40>>1].byte.h = 0x00;  /* Lead-In flags in RS8 (bit0 = mute status, bit1: pre-emphasis status, bit2: track type) */
          break;
        }

        case 0x05:  /* Track Start Time (MM:SS:FF) */
        {
          int track = scd.regs[0x46>>1].byte.h * 10 + scd.regs[0x46>>1].byte.l;
          int lba = cdd.toc.tracks[track-1].start + 150;
          scd.regs[0x38>>1].w = (cdd.status << 8) | 0x05;
          scd.regs[0x3a>>1].w = lut_BCD_16[(lba/75)/60];
          scd.regs[0x3c>>1].w = lut_BCD_16[(lba/75)%60];
          scd.regs[0x3e>>1].w = lut_BCD_16[(lba%75)];
          scd.regs[0x40>>1].byte.h = track % 10;  /* Track Number (low digit) */
          if (track == 1)
          {
            /* RS6 bit 3 is set for the first (DATA) track */
            scd.regs[0x3e>>1].byte.h |= 0x08;
          }
          break;
        }

        default:
        {
#ifdef LOG_ERROR
          error("Unknown CDD Command %02X (%X)\n", scd.regs[0x44>>1].byte.l, s68k.pc);
#endif
          return;
        }
      }
      break;
    }

    case 0x03:  /* Play  */
    {
      /* reset track index */
      int index = 0;

      /* new LBA position */
      int lba = ((scd.regs[0x44>>1].byte.h * 10 + scd.regs[0x44>>1].byte.l) * 60 + 
                 (scd.regs[0x46>>1].byte.h * 10 + scd.regs[0x46>>1].byte.l)) * 75 +
                 (scd.regs[0x48>>1].byte.h * 10 + scd.regs[0x48>>1].byte.l) - 150;

      /* CD drive latency */
      if (!cdd.latency)
      {
        /* Fixes a few games hanging during intro because they expect data to be read with some delay */
        /* Radical Rex needs at least one interrupt delay */
        /* Wolf Team games (Anet Futatabi, Cobra Command, Road Avenger & Time Gal) need at least 6 interrupts delay  */
        /* Space Adventure Cobra (2nd morgue scene) needs at least 13 interrupts delay (incl. seek time, so 6 is OK) */
        /* Jeopardy & ESPN Sunday Night NFL are picky about this as well: 10 interrupts delay (+ seek time) seems OK */
        cdd.latency = 10;
      }

      /* CD drive seek time */
      /* max. seek time = 1.5 s = 1.5 x 75 = 112.5 CDD interrupts (rounded to 120) for 270000 sectors max on disc. */
      /* Note: This is only a rough approximation since, on real hardware, seek time is much likely not linear and */
      /* latency much larger than above value, but this model works fine for Sonic CD (track 26 playback needs to  */
      /* be enough delayed to start in sync with intro sequence, as compared with real hardware recording).        */
      if (lba > cdd.lba)
      {
        cdd.latency += (((lba - cdd.lba) * 120) / 270000);
      }
      else 
      {
        cdd.latency += (((cdd.lba - lba) * 120) / 270000);
      }

      /* update current LBA */
      cdd.lba = lba;

      /* get track index */
      while ((cdd.toc.tracks[index].end <= lba) && (index < cdd.toc.last)) index++;

#if defined(USE_LIBTREMOR) || defined(USE_LIBVORBIS)
#ifdef DISABLE_MANY_OGG_OPEN_FILES
      /* check if track index has changed */
      if (index != cdd.index)
      {
        /* close previous track VORBIS file structure to save memory */
        if (cdd.toc.tracks[cdd.index].vf.datasource)
        {
          ogg_free(cdd.index);
        }

        /* open current track VORBIS file */
        if (cdd.toc.tracks[index].vf.seekable)
        {
          ov_open(cdd.toc.tracks[index].fd,&cdd.toc.tracks[index].vf,0,0);
        }
      }
#endif
#endif

      /* update current track index */
      cdd.index = index;

      /* stay within track limits when seeking files */
      if (lba < cdd.toc.tracks[index].start) 
      {
        lba = cdd.toc.tracks[index].start;
      }
      
      /* seek to current block */
      if (!index)
      {
        /* DATA track */
        fseek(cdd.toc.tracks[0].fd, lba * cdd.sectorSize, SEEK_SET);
      }
#if defined(USE_LIBTREMOR) || defined(USE_LIBVORBIS)
      else if (cdd.toc.tracks[index].vf.seekable)
      {
        /* VORBIS AUDIO track */
        ov_pcm_seek(&cdd.toc.tracks[index].vf, (lba - cdd.toc.tracks[index].start) * 588 - cdd.toc.tracks[index].offset);
      }
#endif 
      else if (cdd.toc.tracks[index].fd)
      {
        /* PCM AUDIO track */
        fseek(cdd.toc.tracks[index].fd, (lba * 2352) - cdd.toc.tracks[index].offset, SEEK_SET);
      }

      /* no audio track playing (yet) */
      scd.regs[0x36>>1].byte.h = 0x01;

      /* update status */
      cdd.status = CD_PLAY;

      /* return track index in RS2-RS3 */
      scd.regs[0x38>>1].w = (CD_PLAY << 8) | 0x02;
      scd.regs[0x3a>>1].w = (cdd.index < cdd.toc.last) ? lut_BCD_16[index + 1] : 0x0A0A;
      scd.regs[0x3c>>1].w = 0x0000;
      scd.regs[0x3e>>1].w = 0x0000;
      scd.regs[0x40>>1].byte.h = 0x00;
      break;
    }

    case 0x04:  /* Seek */
    {
      /* reset track index */
      int index = 0;

      /* new LBA position */
      int lba = ((scd.regs[0x44>>1].byte.h * 10 + scd.regs[0x44>>1].byte.l) * 60 + 
                 (scd.regs[0x46>>1].byte.h * 10 + scd.regs[0x46>>1].byte.l)) * 75 +
                 (scd.regs[0x48>>1].byte.h * 10 + scd.regs[0x48>>1].byte.l) - 150;

      /* CD drive seek time  */
      /* We are using similar linear model as above, although still not exactly accurate, */
      /* it works fine for Switch/Panic! intro (Switch needs at least 30 interrupts while */
      /* seeking from 00:05:63 to 24:03:19, Panic! when seeking from 00:05:60 to 24:06:07) */
      if (lba > cdd.lba)
      {
        cdd.latency = ((lba - cdd.lba) * 120) / 270000;
      }
      else
      {
        cdd.latency = ((cdd.lba - lba) * 120) / 270000;
      }

      /* update current LBA */
      cdd.lba = lba;

      /* get current track index */
      while ((cdd.toc.tracks[index].end <= lba) && (index < cdd.toc.last)) index++;

#if defined(USE_LIBTREMOR) || defined(USE_LIBVORBIS)
#ifdef DISABLE_MANY_OGG_OPEN_FILES
      /* check if track index has changed */
      if (index != cdd.index)
      {
        /* close previous track VORBIS file structure to save memory */
        if (cdd.toc.tracks[cdd.index].vf.datasource)
        {
          ogg_free(cdd.index);
        }

        /* open current track VORBIS file */
        if (cdd.toc.tracks[index].vf.seekable)
        {
          ov_open(cdd.toc.tracks[index].fd,&cdd.toc.tracks[index].vf,0,0);
        }
      }
#endif
#endif

      /* update current track index */
      cdd.index = index;

      /* stay within track limits */
      if (lba < cdd.toc.tracks[index].start) 
      {
        lba = cdd.toc.tracks[index].start;
      }
      
      /* seek to current block */
      if (!index)
      {
        /* DATA track */
        fseek(cdd.toc.tracks[0].fd, lba * cdd.sectorSize, SEEK_SET);
      }
#if defined(USE_LIBTREMOR) || defined(USE_LIBVORBIS)
      else if (cdd.toc.tracks[index].vf.seekable)
      {
        /* VORBIS AUDIO track */
        ov_pcm_seek(&cdd.toc.tracks[index].vf, (lba - cdd.toc.tracks[index].start) * 588 - cdd.toc.tracks[index].offset);
      }
#endif 
      else if (cdd.toc.tracks[index].fd)
      {
        /* PCM AUDIO track */
        fseek(cdd.toc.tracks[index].fd, (lba * 2352) - cdd.toc.tracks[index].offset, SEEK_SET);
      }

      /* no audio track playing */
      scd.regs[0x36>>1].byte.h = 0x01;

      /* update status */
      cdd.status = CD_SEEK;

      /* unknown RS1-RS8 values (returning 0xF in RS1 invalidates track infos in RS2-RS8 and fixes Final Fight CD intro when seek time is emulated) */
      scd.regs[0x38>>1].w = (CD_SEEK << 8) | 0x0f;
      scd.regs[0x3a>>1].w = 0x0000;
      scd.regs[0x3c>>1].w = 0x0000;
      scd.regs[0x3e>>1].w = 0x0000;
      scd.regs[0x40>>1].w = ~(CD_SEEK + 0xf) & 0x0f;
      return;
    }

    case 0x06:  /* Pause */
    {
      /* no audio track playing */
      scd.regs[0x36>>1].byte.h = 0x01;

      /* update status (RS1-RS8 unchanged) */
      cdd.status = scd.regs[0x38>>1].byte.h = CD_READY;
      break;
    }

    case 0x07:  /* Resume */
    {
      /* update status (RS1-RS8 unchanged) */
      cdd.status = scd.regs[0x38>>1].byte.h = CD_PLAY;
      break;
    }

    case 0x08:  /* Forward Scan */
    {
      /* reset scanning direction / speed */
      cdd.scanOffset = CD_SCAN_SPEED;

      /* update status (RS1-RS8 unchanged) */
      cdd.status = scd.regs[0x38>>1].byte.h = CD_SCAN;
      break;
    }

    case 0x09:  /* Rewind Scan */
    {
      /* reset scanning direction / speed */
      cdd.scanOffset = -CD_SCAN_SPEED;

      /* update status (RS1-RS8 unchanged) */
      cdd.status = scd.regs[0x38>>1].byte.h = CD_SCAN;
      break;
    }


    case 0x0a:  /* N-Track Jump Control ? (usually sent before CD_SEEK or CD_PLAY commands) */
    {
      /* TC3 corresponds to seek direction (00=forward, FF=reverse) */
      /* TC4-TC7 are related to seek length (4x4 bits i.e parameter values are between -65535 and +65535) */
      /* Maybe related to number of auto-sequenced track jumps/moves for CD DSP (cf. CXD2500BQ datasheet) */
      /* also see US Patent nr. 5222054 for a detailled description of seeking operation using Track Jump */

      /* no audio track playing */
      scd.regs[0x36>>1].byte.h = 0x01;

      /* update status (RS1-RS8 unchanged) */
      cdd.status = scd.regs[0x38>>1].byte.h = CD_READY;
      break;
    }

    case 0x0c:  /* Close Tray */
    {
      /* no audio track playing */
      scd.regs[0x36>>1].byte.h = 0x01;

      /* update status */
      cdd.status = cdd.loaded ? CD_STOP : NO_DISC;

      /* RS1-RS8 ignored, expects 0x0 ("no disc" ?) in RS0 once */
      scd.regs[0x38>>1].w = 0x0000;
      scd.regs[0x3a>>1].w = 0x0000;
      scd.regs[0x3c>>1].w = 0x0000;
      scd.regs[0x3e>>1].w = 0x0000;
      scd.regs[0x40>>1].w = 0x000f;

#ifdef CD_TRAY_CALLBACK
      CD_TRAY_CALLBACK
#endif
      return;
    }

    case 0x0d:  /* Open Tray */
    {
      /* no audio track playing */
      scd.regs[0x36>>1].byte.h = 0x01;

      /* update status (RS1-RS8 ignored) */
      cdd.status = CD_OPEN;
      scd.regs[0x38>>1].w = CD_OPEN << 8;
      scd.regs[0x3a>>1].w = 0x0000;
      scd.regs[0x3c>>1].w = 0x0000;
      scd.regs[0x3e>>1].w = 0x0000;
      scd.regs[0x40>>1].w = ~CD_OPEN & 0x0f;

#ifdef CD_TRAY_CALLBACK
      CD_TRAY_CALLBACK
#endif
      return;
    }

    default:  /* Unknown command */
#ifdef LOG_CDD
      error("Unknown CDD Command !!!\n");
#endif
      scd.regs[0x38>>1].byte.h = cdd.status;
      break;
  }

  /* only compute checksum when necessary */
  scd.regs[0x40>>1].byte.l = ~(scd.regs[0x38>>1].byte.h + scd.regs[0x38>>1].byte.l +
                               scd.regs[0x3a>>1].byte.h + scd.regs[0x3a>>1].byte.l +
                               scd.regs[0x3c>>1].byte.h + scd.regs[0x3c>>1].byte.l +
                               scd.regs[0x3e>>1].byte.h + scd.regs[0x3e>>1].byte.l +
                               scd.regs[0x40>>1].byte.h) & 0x0f;
}
