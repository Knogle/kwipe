# Compiler-Flags und Linker-Flags, die verwendet werden sollen
AM_CFLAGS = -I$(srcdir)/include/SFMT-src-1.5.1 -DHAVE_SSE2 -DSFMT_MEXP=19937 -msse2
AM_LDFLAGS = 

# Liste der Binaries, die erstellt werden sollen
bin_PROGRAMS = nwipe

# Quellen, die zum Erstellen von 'nwipe' benötigt werden
nwipe_SOURCES = context.h logging.h options.h prng.h version.h temperature.h \
                nwipe.c gui.c method.h pass.c device.c gui.h isaac_rand/isaac_standard.h \
                isaac_rand/isaac_rand.h isaac_rand/isaac_rand.c isaac_rand/isaac64.h \
                isaac_rand/isaac64.c mt19937ar-cok/mt19937ar-cok.c nwipe.h \
                mt19937ar-cok/mt19937ar-cok.h alfg/add_lagg_fibonacci_prng.h \
                alfg/add_lagg_fibonacci_prng.c xor/xoroshiro256_prng.h \
                xor/xoroshiro256_prng.c pass.h device.h logging.c method.c options.c \
                prng.c version.c temperature.c PDFGen/pdfgen.h PDFGen/pdfgen.c \
                create_pdf.c create_pdf.h embedded_images/shred_db.jpg.c \
                embedded_images/shred_db.jpg.h embedded_images/tick_erased.jpg.c \
                embedded_images/tick_erased.jpg.h embedded_images/redcross.c \
                embedded_images/redcross.h hpa_dco.h hpa_dco.c miscellaneous.h \
                miscellaneous.c embedded_images/nwipe_exclamation.jpg.h \
                embedded_images/nwipe_exclamation.jpg.c conf.h conf.c customers.h \
                customers.c hddtemp_scsi/hddtemp.h hddtemp_scsi/scsi.h \
                hddtemp_scsi/scsicmds.h hddtemp_scsi/get_scsi_temp.c \
                hddtemp_scsi/scsi.c hddtemp_scsi/scsicmds.c \
                include/SFMT-src-1.5.1/SFMT.h include/SFMT-src-1.5.1/SFMT.c \
                include/SFMT-src-1.5.1/SFMT-sse2.h include/SFMT-src-1.5.1/SFMT-common.h

# Bibliotheken, die während der Verlinkung hinzugefügt werden müssen
nwipe_LDADD = $(PARTED_LIBS) $(LIBCONFIG)

