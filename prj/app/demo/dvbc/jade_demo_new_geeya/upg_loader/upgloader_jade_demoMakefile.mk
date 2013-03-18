
##############################################
#
#    Makefile for your App
#        Ali sdk auto generated. Don't edit it!
#                            ALI corporation.
#
##############################################


# Environment settings
GCCROOT = /cygdrive/D/study/v2.1/ALi_IDE_2.1_compile_tools/compilers/gcc3.0.4/
export PATH:=/bin:/cygdrive/C/Ali_SDK/bin/:$(PATH)

# gcc prefix
Prefix = mips-t2-elf-

CC = $(GCCROOT)bin/$(Prefix)gcc.exe
CXX = $(GCCROOT)bin/$(Prefix)g++.exe
AR = $(GCCROOT)bin/$(Prefix)ar.exe
LD = $(GCCROOT)bin/$(Prefix)ld.exe
OBJCOPY = $(GCCROOT)bin/$(Prefix)objcopy.exe
OBJDUMP = $(GCCROOT)bin/$(Prefix)objdump.exe

CP = cp
ECHO = echo
MAKE = make

# Parameters
ccflag =  -O1 -mips2 -EL -D_DEBUG_VERSION_ -DALi_M6303_Module_1 -W -I$(release_path)/include/ -g -msoft-float -fno-delayed-branch -I$(release_path)/include/ -I$(REPOSITORY)/inc
ldflag = -O1  -EL -defsym __BOOT_FROM_FLASH=0 -defsym __BOOT_LOADER=0 -T ldscript.ld -lall -lbootos -lcore -lall -llog -lmlzo -l7zip -lm -lgcc -lc -L$(release_path)/lib/ -Map ${BINNAME}.map -L$(REPOSITORY)/sdk -L$(REPOSITORY)/sdk/blsdk -L$(GCCROOT)lib/gcc-lib/mips-t2-elf/3.0.4/soft-float/el -L$(GCCROOT)mips-t2-elf/lib/soft-float/el
# Repository path which contains the root module,
# is root for all modules' source files.
REPOSITORY = /cygdrive/d/sdk4.0ad.1.3_jade_c_20121207_16bit_new_new/

# release_path is module library's root,
# It has two subdirectories: include and lib.
release_path = /cygdrive/d/sdk4.0ad.1.3_jade_c_20121207_16bit_new_new/prj/app/demo/dvbc/jade_demo_new_16bit/upg_loader/mcf//release

# RESOURCE_BUILD_PATH is resources build path,
# stores .h files which built from resources.
RESOURCE_BUILD_PATH = MCF/Resource

# APP_ROOT_PATH is application root path,
# all app files are under this root.
APP_ROOT_PATH = /cygdrive/d/sdk4.0ad.1.3_jade_c_20121207_16bit_new_new/prj/app/demo/dvbc/jade_demo_new_16bit/upg_loader/

# project release path.
RELEASE_PROJECT_PATH = xxx

# Custom Predefine:  

# List of source files

ASMS = \


SRCS = \
	../copper_common/stb_data.c \
	../copper_common/stb_hwinfo.c \
	../copper_common/backup_loader.c \
	boot_osd_rsc.c \
	key.c \
	boot_ota_input.c \
	upgloader.c \
	upg_common.c \
	upg_ota.c \
	upg_menu.c \
	board_config.c

SRCPPS = \


XFORMS = \


XFORMHS = ${XFORMS:.xform=.h}


OBJS = ${ASMS:.S=.o} ${SRCS:.c=.o} ${SRCPPS:.cpp=.o}

# Set binary file name
BINNAME = upgloader_jade_demo


.PHONY : prelink
prelink:

.PHONY : all
all : prelink labelMakefile.mk
	$(MAKE) -f $(BINNAME)Makefile.mk $(OBJS)
	$(LD) $(OBJS) $(ldflag) -o $(BINNAME).exe
	-$(CP) -f $(BINNAME).exe $(BINNAME).out
	$(OBJCOPY) -O binary $(BINNAME).exe $(BINNAME).abs
	$(OBJDUMP) -d $(BINNAME).exe > $(BINNAME).dis
	@echo compress :
	${OBJCOPY} -O binary ${BINNAME}.out ${BINNAME}_tmp; \
	${OBJDUMP} -D ${BINNAME}.out > ${BINNAME}.dis ;\
	rm -f ${BINNAME}.7z; \
	${REPOSITORY}/prj/tools/lzma e ${BINNAME}_tmp ${BINNAME}.7z -lc0 -lp2
		
	@echo generate :
	# .flash address - 65552 = 221168
	rm -f app_${BINNAME}.bin
	rm -f upg_common.o upgloader.o stb_data.o upg_usb.o
	cp ${BINNAME}.7z ../${BINNAME}.bin
	rm -f ${BINNAME}.7z

.PHONY : app_as_lib
app_as_lib : prelink labelMakefile.mk
	$(MAKE) -f $(BINNAME)Makefile.mk $(OBJS)
	$(AR) rcs $(BINNAME).a $(OBJS)
	@echo build complete.

.PHONY : debug
debug : prelink labelMakefile.mk
	$(MAKE) -f $(BINNAME)Makefile.mk $(OBJS)
	$(LD) $(OBJS) $(ldflag) -o $(BINNAME).exe
	-$(CP) -f $(BINNAME).exe $(BINNAME).out
	$(OBJCOPY) -O binary $(BINNAME).exe $(BINNAME).abs
	@echo build complete.

.PHONY : labelMakefile.mk
labelMakefile.mk : $(SRCS) #$(ASMS)
	$(CP) -f $(BINNAME)Makefile $(BINNAME)Makefile.mk
	-$(CC) $(ccflag) -M $(SRCS) $(SRCPPS) $(ASMS) >> $@.tmp
	-modify_dep.exe $@.tmp
	-cat $@.tmp >> $(BINNAME)Makefile.mk
	-rm $@.tmp

.PHONY : check
check : 
	@-splint.exe -strict -gnuextensions -DLINT  -D_DEBUG_VERSION_ -DALi_M6303_Module_1 -I$(release_path)/include/ -I$(RESOURCE_BUILD_PATH) -I$(release_path)/include/ -I$(REPOSITORY)/inc/ $(SRCS)
	@echo check complete.


.PHONY : clean
clean : 
	@-rm -f $(OBJS)
	@-rm -f $(BINNAME)Makefile.mk
	@-rm -f $(BINNAME).exe
	#@-rm -f $(BINNAME).abs
	@-rm -f $(BINNAME).dis
	@echo clean complete.


.PHONY : dependency
dependency : prelink labelMakefile.mk
	@echo make dependency complete.


.PHONY : custombat
custombat :

	@echo custom bat cmds complete.


%.o : %.c
	@echo $<
	$(CC) $(ccflag) -c $< -g -o $@

%.o : %.S
	@echo $<
	$(CC) $(ccflag) -c $< -g -o $@

%.o : %.cpp
	@echo $<
	$(CXX) $(ccflag) -c $< -g -o $@

%.h : %.xform
	${REPOSITORY}/prj/tools/xform2c $< $@


.PHONY : release
release:
	dos2unix prj_rel.sh
	echo 'REPOSITORY=$(REPOSITORY)' > $(BINNAME)Makefile.rel
	echo 'APP_ROOT_PATH=$(APP_ROOT_PATH)' >> $(BINNAME)Makefile.rel
	echo 'RELEASE_PATH=$(RELEASE_PROJECT_PATH)' >> $(BINNAME)Makefile.rel
	-$(CC) $(ccflag) -M $(SRCS) $(SRCPPS) $(ASMS) > $@.tmp
	sed "/[^\\\\]$$/a\\	@echo \$$<;export REPOSITORY=\$$(REPOSITORY); export APP_ROOT_PATH=\$$(APP_ROOT_PATH);export RELEASE_PATH=\$$(RELEASE_PATH); ./prj_rel.sh $$+" $@.tmp >> $(BINNAME)Makefile.rel
	-rm $@.tmp
	echo "others.o:" upgloader_jade_demo.apr >>  $(BINNAME)Makefile.rel
	echo "	export REPOSITORY=\$$(REPOSITORY); export APP_ROOT_PATH=\$$(APP_ROOT_PATH);export RELEASE_PATH=\$$(RELEASE_PATH); ./prj_rel.sh upgloader_jade_demo.apr ldscript.ld $(release_path)/lib//libbootos.a $(release_path)/lib//libcore.a $(release_path)/lib//liblog.a $(release_path)/lib//libmlzo.a $(release_path)/lib//lib7zip.a $(REPOSITORY)/sdk/libbootos.a $(REPOSITORY)/sdk/libcore.a $(REPOSITORY)/sdk/liblog.a $(REPOSITORY)/sdk/libmlzo.a $(REPOSITORY)/sdk/lib7zip.a $(REPOSITORY)/sdk/blsdk/libbootos.a $(REPOSITORY)/sdk/blsdk/libcore.a $(REPOSITORY)/sdk/blsdk/liblog.a $(REPOSITORY)/sdk/blsdk/libmlzo.a $(REPOSITORY)/sdk/blsdk/lib7zip.a /cygdrive/d/sdk4.0ad.1.3_jade_c_20121207_16bit_new_new/sdk/boot.o *.ini flash*.abs" >> $(BINNAME)Makefile.rel
	echo "	mkdir -p \$$(RELEASE_PATH)/prj/tools; cp -f \$$(REPOSITORY)/prj/tools/*  \$$(RELEASE_PATH)/prj/tools/" >> $(BINNAME)Makefile.rel
	echo "release: stb_data.o stb_hwinfo.o backup_loader.o boot_osd_rsc.o key.o boot_ota_input.o upgloader.o upg_common.o upg_ota.o upg_menu.o board_config.o others.o" >> $(BINNAME)Makefile.rel
	echo "	@echo release done!" >> $(BINNAME)Makefile.rel
	make -sf $(BINNAME)Makefile.rel release
	-rm -f $(BINNAME)Makefile.rel
	-rm -f prj_rel.sh
	# Make File End

stb_data.o: ../copper_common/stb_data.c \
  /cygdrive/d/sdk4.0ad.1.3_jade_c_20121207_16bit_new_new/prj/app/demo/dvbc/jade_demo_new_16bit/upg_loader/mcf/release/include/sys_config.h \
  /cygdrive/d/sdk4.0ad.1.3_jade_c_20121207_16bit_new_new/inc/sys_define.h \
  /cygdrive/d/sdk4.0ad.1.3_jade_c_20121207_16bit_new_new/prj/app/demo/dvbc/jade_demo_new_16bit/upg_loader/mcf/release/include/common_config.h \
  ../copper_common/dev_handle.h ../copper_common/boot_system_data.h \
  ../copper_common/stb_data.h \
  /cygdrive/d/sdk4.0ad.1.3_jade_c_20121207_16bit_new_new/inc/types.h \
  /cygdrive/d/sdk4.0ad.1.3_jade_c_20121207_16bit_new_new/inc/basic_types.h \
  /cygdrive/d/sdk4.0ad.1.3_jade_c_20121207_16bit_new_new/inc/mediatypes.h \
  /cygdrive/D/study/v2.1/ALi_IDE_2.1_compile_tools/compilers/gcc3.0.4/mips-t2-elf/include/math.h \
  /cygdrive/D/study/v2.1/ALi_IDE_2.1_compile_tools/compilers/gcc3.0.4/mips-t2-elf/include/sys/reent.h \
  /cygdrive/D/study/v2.1/ALi_IDE_2.1_compile_tools/compilers/gcc3.0.4/mips-t2-elf/include/_ansi.h \
  /cygdrive/D/study/v2.1/ALi_IDE_2.1_compile_tools/compilers/gcc3.0.4/mips-t2-elf/include/sys/config.h \
  /cygdrive/D/study/v2.1/ALi_IDE_2.1_compile_tools/compilers/gcc3.0.4/mips-t2-elf/include/time.h \
  /cygdrive/D/study/v2.1/ALi_IDE_2.1_compile_tools/compilers/gcc3.0.4/mips-t2-elf/include/machine/time.h \
  /cygdrive/D/study/v2.1/ALi_IDE_2.1_compile_tools/compilers/gcc3.0.4/lib/gcc-lib/mips-t2-elf/3.0.4/include/stddef.h \
  /cygdrive/D/study/v2.1/ALi_IDE_2.1_compile_tools/compilers/gcc3.0.4/mips-t2-elf/include/sys/types.h \
  /cygdrive/D/study/v2.1/ALi_IDE_2.1_compile_tools/compilers/gcc3.0.4/mips-t2-elf/include/machine/types.h \
  /cygdrive/D/study/v2.1/ALi_IDE_2.1_compile_tools/compilers/gcc3.0.4/mips-t2-elf/include/sys/features.h \
  /cygdrive/D/study/v2.1/ALi_IDE_2.1_compile_tools/compilers/gcc3.0.4/mips-t2-elf/include/machine/ieeefp.h \
  /cygdrive/d/sdk4.0ad.1.3_jade_c_20121207_16bit_new_new/inc/osal/osal.h \
  /cygdrive/d/sdk4.0ad.1.3_jade_c_20121207_16bit_new_new/inc/retcode.h \
  /cygdrive/d/sdk4.0ad.1.3_jade_c_20121207_16bit_new_new/inc/os/tds2/itron.h \
  /cygdrive/d/sdk4.0ad.1.3_jade_c_20121207_16bit_new_new/inc/os/tds2/alitypes.h \
  /cygdrive/d/sdk4.0ad.1.3_jade_c_20121207_16bit_new_new/inc/os/tds2/platform.h \
  /cygdrive/d/sdk4.0ad.1.3_jade_c_20121207_16bit_new_new/inc/os/tds2/config.h \
  /cygdrive/d/sdk4.0ad.1.3_jade_c_20121207_16bit_new_new/inc/api/libc/printf.h \
  /cygdrive/d/sdk4.0ad.1.3_jade_c_20121207_16bit_new_new/inc/hal/machine.h \
  /cygdrive/D/study/v2.1/ALi_IDE_2.1_compile_tools/compilers/gcc3.0.4/lib/gcc-lib/mips-t2-elf/3.0.4/include/stdarg.h \
  /cygdrive/d/sdk4.0ad.1.3_jade_c_20121207_16bit_new_new/inc/osal/osal_task.h \
  /cygdrive/d/sdk4.0ad.1.3_jade_c_20121207_16bit_new_new/inc/osal/osal_int.h \
  /cygdrive/d/sdk4.0ad.1.3_jade_c_20121207_16bit_new_new/inc/osal/osal_timer.h \
  /cygdrive/d/sdk4.0ad.1.3_jade_c_20121207_16bit_new_new/inc/osal/osal_msgq.h \
  /cygdrive/d/sdk4.0ad.1.3_jade_c_20121207_16bit_new_new/inc/osal/osal_sema.h \
  /cygdrive/d/sdk4.0ad.1.3_jade_c_20121207_16bit_new_new/inc/osal/osal_flag.h \
  /cygdrive/d/sdk4.0ad.1.3_jade_c_20121207_16bit_new_new/inc/osal/osal_mm.h \
  /cygdrive/d/sdk4.0ad.1.3_jade_c_20121207_16bit_new_new/inc/api/libc/alloc.h \
  /cygdrive/d/sdk4.0ad.1.3_jade_c_20121207_16bit_new_new/inc/osal/osal_cache.h \
  /cygdrive/d/sdk4.0ad.1.3_jade_c_20121207_16bit_new_new/inc/osal/osal_mutex.h \
  /cygdrive/d/sdk4.0ad.1.3_jade_c_20121207_16bit_new_new/inc/api/libc/string.h \
  /cygdrive/d/sdk4.0ad.1.3_jade_c_20121207_16bit_new_new/inc/api/libstbinfo/stb_info.h \
  /cygdrive/d/sdk4.0ad.1.3_jade_c_20121207_16bit_new_new/inc/api/libstbinfo/stb_info_data.h \
  /cygdrive/d/sdk4.0ad.1.3_jade_c_20121207_16bit_new_new/inc/api/libloader/ld_boot_info.h \
  /cygdrive/d/sdk4.0ad.1.3_jade_c_20121207_16bit_new_new/inc/api/libpub/lib_frontend.h \
  /cygdrive/d/sdk4.0ad.1.3_jade_c_20121207_16bit_new_new/inc/api/libtsi/p_search.h \
  /cygdrive/d/sdk4.0ad.1.3_jade_c_20121207_16bit_new_new/inc/api/libsi/si_config.h \
  /cygdrive/d/sdk4.0ad.1.3_jade_c_20121207_16bit_new_new/inc/api/libtsi/sec_pmt.h \
  /cygdrive/d/sdk4.0ad.1.3_jade_c_20121207_16bit_new_new/inc/api/libtsi/sec_sdt.h \
  /cygdrive/d/sdk4.0ad.1.3_jade_c_20121207_16bit_new_new/inc/api/libtsi/db_3l.h \
  /cygdrive/d/sdk4.0ad.1.3_jade_c_20121207_16bit_new_new/inc/api/libdb/db_return_value.h \
  /cygdrive/d/sdk4.0ad.1.3_jade_c_20121207_16bit_new_new/inc/api/libdb/db_config.h \
  /cygdrive/d/sdk4.0ad.1.3_jade_c_20121207_16bit_new_new/inc/api/libdb/db_node_c.h \
  /cygdrive/d/sdk4.0ad.1.3_jade_c_20121207_16bit_new_new/inc/api/libdb/db_interface.h \
  /cygdrive/d/sdk4.0ad.1.3_jade_c_20121207_16bit_new_new/inc/api/libchunk/chunk.h \
  /cygdrive/d/sdk4.0ad.1.3_jade_c_20121207_16bit_new_new/inc/hld/sto/sto.h \
  /cygdrive/d/sdk4.0ad.1.3_jade_c_20121207_16bit_new_new/inc/hld/sto/sto_dev.h \
  /cygdrive/d/sdk4.0ad.1.3_jade_c_20121207_16bit_new_new/inc/hld/hld_dev.h \
  /cygdrive/d/sdk4.0ad.1.3_jade_c_20121207_16bit_new_new/inc/bus/erom/erom.h
stb_hwinfo.o: ../copper_common/stb_hwinfo.c \
  /cygdrive/d/sdk4.0ad.1.3_jade_c_20121207_16bit_new_new/prj/app/demo/dvbc/jade_demo_new_16bit/upg_loader/mcf/release/include/sys_config.h \
  /cygdrive/d/sdk4.0ad.1.3_jade_c_20121207_16bit_new_new/inc/sys_define.h \
  /cygdrive/d/sdk4.0ad.1.3_jade_c_20121207_16bit_new_new/prj/app/demo/dvbc/jade_demo_new_16bit/upg_loader/mcf/release/include/common_config.h \
  /cygdrive/D/study/v2.1/ALi_IDE_2.1_compile_tools/compilers/gcc3.0.4/lib/gcc-lib/mips-t2-elf/3.0.4/include/stdarg.h \
  /cygdrive/D/study/v2.1/ALi_IDE_2.1_compile_tools/compilers/gcc3.0.4/lib/gcc-lib/mips-t2-elf/3.0.4/include/stddef.h \
  /cygdrive/d/sdk4.0ad.1.3_jade_c_20121207_16bit_new_new/inc/osal/osal.h \
  /cygdrive/d/sdk4.0ad.1.3_jade_c_20121207_16bit_new_new/inc/retcode.h \
  /cygdrive/d/sdk4.0ad.1.3_jade_c_20121207_16bit_new_new/inc/types.h \
  /cygdrive/d/sdk4.0ad.1.3_jade_c_20121207_16bit_new_new/inc/basic_types.h \
  /cygdrive/d/sdk4.0ad.1.3_jade_c_20121207_16bit_new_new/inc/os/tds2/itron.h \
  /cygdrive/d/sdk4.0ad.1.3_jade_c_20121207_16bit_new_new/inc/os/tds2/alitypes.h \
  /cygdrive/d/sdk4.0ad.1.3_jade_c_20121207_16bit_new_new/inc/os/tds2/platform.h \
  /cygdrive/d/sdk4.0ad.1.3_jade_c_20121207_16bit_new_new/inc/os/tds2/config.h \
  /cygdrive/d/sdk4.0ad.1.3_jade_c_20121207_16bit_new_new/inc/api/libc/printf.h \
  /cygdrive/d/sdk4.0ad.1.3_jade_c_20121207_16bit_new_new/inc/hal/machine.h \
  /cygdrive/d/sdk4.0ad.1.3_jade_c_20121207_16bit_new_new/inc/osal/osal_task.h \
  /cygdrive/d/sdk4.0ad.1.3_jade_c_20121207_16bit_new_new/inc/osal/osal_int.h \
  /cygdrive/d/sdk4.0ad.1.3_jade_c_20121207_16bit_new_new/inc/osal/osal_timer.h \
  /cygdrive/d/sdk4.0ad.1.3_jade_c_20121207_16bit_new_new/inc/osal/osal_msgq.h \
  /cygdrive/d/sdk4.0ad.1.3_jade_c_20121207_16bit_new_new/inc/osal/osal_sema.h \
  /cygdrive/d/sdk4.0ad.1.3_jade_c_20121207_16bit_new_new/inc/osal/osal_flag.h \
  /cygdrive/d/sdk4.0ad.1.3_jade_c_20121207_16bit_new_new/inc/osal/osal_mm.h \
  /cygdrive/d/sdk4.0ad.1.3_jade_c_20121207_16bit_new_new/inc/api/libc/alloc.h \
  /cygdrive/d/sdk4.0ad.1.3_jade_c_20121207_16bit_new_new/inc/osal/osal_cache.h \
  /cygdrive/d/sdk4.0ad.1.3_jade_c_20121207_16bit_new_new/inc/osal/osal_mutex.h \
  /cygdrive/d/sdk4.0ad.1.3_jade_c_20121207_16bit_new_new/inc/api/libc/string.h \
  /cygdrive/d/sdk4.0ad.1.3_jade_c_20121207_16bit_new_new/inc/bus/sci/sci.h \
  /cygdrive/d/sdk4.0ad.1.3_jade_c_20121207_16bit_new_new/inc/hld/hld_dev.h \
  /cygdrive/d/sdk4.0ad.1.3_jade_c_20121207_16bit_new_new/inc/hld/sto/sto_dev.h \
  /cygdrive/d/sdk4.0ad.1.3_jade_c_20121207_16bit_new_new/inc/bus/erom/erom.h \
  ../copper_common/stb_hwinfo.h ../key.h \
  /cygdrive/d/sdk4.0ad.1.3_jade_c_20121207_16bit_new_new/src/lib/libapplet/libbootupg3/lib_ash.h \
  /cygdrive/d/sdk4.0ad.1.3_jade_c_20121207_16bit_new_new/src/lib/libapplet/libbootupg3/errorcode.h \
  /cygdrive/d/sdk4.0ad.1.3_jade_c_20121207_16bit_new_new/src/lib/libapplet/libbootupg3/packet.h
backup_loader.o: ../copper_common/backup_loader.c \
  /cygdrive/d/sdk4.0ad.1.3_jade_c_20121207_16bit_new_new/prj/app/demo/dvbc/jade_demo_new_16bit/upg_loader/mcf/release/include/sys_config.h \
  /cygdrive/d/sdk4.0ad.1.3_jade_c_20121207_16bit_new_new/inc/sys_define.h \
  /cygdrive/d/sdk4.0ad.1.3_jade_c_20121207_16bit_new_new/prj/app/demo/dvbc/jade_demo_new_16bit/upg_loader/mcf/release/include/common_config.h \
  /cygdrive/d/sdk4.0ad.1.3_jade_c_20121207_16bit_new_new/inc/types.h \
  /cygdrive/d/sdk4.0ad.1.3_jade_c_20121207_16bit_new_new/inc/basic_types.h \
  /cygdrive/d/sdk4.0ad.1.3_jade_c_20121207_16bit_new_new/inc/mediatypes.h \
  /cygdrive/D/study/v2.1/ALi_IDE_2.1_compile_tools/compilers/gcc3.0.4/mips-t2-elf/include/math.h \
  /cygdrive/D/study/v2.1/ALi_IDE_2.1_compile_tools/compilers/gcc3.0.4/mips-t2-elf/include/sys/reent.h \
  /cygdrive/D/study/v2.1/ALi_IDE_2.1_compile_tools/compilers/gcc3.0.4/mips-t2-elf/include/_ansi.h \
  /cygdrive/D/study/v2.1/ALi_IDE_2.1_compile_tools/compilers/gcc3.0.4/mips-t2-elf/include/sys/config.h \
  /cygdrive/D/study/v2.1/ALi_IDE_2.1_compile_tools/compilers/gcc3.0.4/mips-t2-elf/include/time.h \
  /cygdrive/D/study/v2.1/ALi_IDE_2.1_compile_tools/compilers/gcc3.0.4/mips-t2-elf/include/machine/time.h \
  /cygdrive/D/study/v2.1/ALi_IDE_2.1_compile_tools/compilers/gcc3.0.4/lib/gcc-lib/mips-t2-elf/3.0.4/include/stddef.h \
  /cygdrive/D/study/v2.1/ALi_IDE_2.1_compile_tools/compilers/gcc3.0.4/mips-t2-elf/include/sys/types.h \
  /cygdrive/D/study/v2.1/ALi_IDE_2.1_compile_tools/compilers/gcc3.0.4/mips-t2-elf/include/machine/types.h \
  /cygdrive/D/study/v2.1/ALi_IDE_2.1_compile_tools/compilers/gcc3.0.4/mips-t2-elf/include/sys/features.h \
  /cygdrive/D/study/v2.1/ALi_IDE_2.1_compile_tools/compilers/gcc3.0.4/mips-t2-elf/include/machine/ieeefp.h \
  /cygdrive/d/sdk4.0ad.1.3_jade_c_20121207_16bit_new_new/inc/osal/osal.h \
  /cygdrive/d/sdk4.0ad.1.3_jade_c_20121207_16bit_new_new/inc/retcode.h \
  /cygdrive/d/sdk4.0ad.1.3_jade_c_20121207_16bit_new_new/inc/os/tds2/itron.h \
  /cygdrive/d/sdk4.0ad.1.3_jade_c_20121207_16bit_new_new/inc/os/tds2/alitypes.h \
  /cygdrive/d/sdk4.0ad.1.3_jade_c_20121207_16bit_new_new/inc/os/tds2/platform.h \
  /cygdrive/d/sdk4.0ad.1.3_jade_c_20121207_16bit_new_new/inc/os/tds2/config.h \
  /cygdrive/d/sdk4.0ad.1.3_jade_c_20121207_16bit_new_new/inc/api/libc/printf.h \
  /cygdrive/d/sdk4.0ad.1.3_jade_c_20121207_16bit_new_new/inc/hal/machine.h \
  /cygdrive/D/study/v2.1/ALi_IDE_2.1_compile_tools/compilers/gcc3.0.4/lib/gcc-lib/mips-t2-elf/3.0.4/include/stdarg.h \
  /cygdrive/d/sdk4.0ad.1.3_jade_c_20121207_16bit_new_new/inc/osal/osal_task.h \
  /cygdrive/d/sdk4.0ad.1.3_jade_c_20121207_16bit_new_new/inc/osal/osal_int.h \
  /cygdrive/d/sdk4.0ad.1.3_jade_c_20121207_16bit_new_new/inc/osal/osal_timer.h \
  /cygdrive/d/sdk4.0ad.1.3_jade_c_20121207_16bit_new_new/inc/osal/osal_msgq.h \
  /cygdrive/d/sdk4.0ad.1.3_jade_c_20121207_16bit_new_new/inc/osal/osal_sema.h \
  /cygdrive/d/sdk4.0ad.1.3_jade_c_20121207_16bit_new_new/inc/osal/osal_flag.h \
  /cygdrive/d/sdk4.0ad.1.3_jade_c_20121207_16bit_new_new/inc/osal/osal_mm.h \
  /cygdrive/d/sdk4.0ad.1.3_jade_c_20121207_16bit_new_new/inc/api/libc/alloc.h \
  /cygdrive/d/sdk4.0ad.1.3_jade_c_20121207_16bit_new_new/inc/osal/osal_cache.h \
  /cygdrive/d/sdk4.0ad.1.3_jade_c_20121207_16bit_new_new/inc/osal/osal_mutex.h \
  /cygdrive/d/sdk4.0ad.1.3_jade_c_20121207_16bit_new_new/inc/api/libc/string.h \
  /cygdrive/d/sdk4.0ad.1.3_jade_c_20121207_16bit_new_new/inc/bus/flash/flash.h \
  /cygdrive/d/sdk4.0ad.1.3_jade_c_20121207_16bit_new_new/inc/api/libchunk/chunk.h \
  /cygdrive/d/sdk4.0ad.1.3_jade_c_20121207_16bit_new_new/inc/hld/sto/sto.h \
  /cygdrive/d/sdk4.0ad.1.3_jade_c_20121207_16bit_new_new/inc/hld/sto/sto_dev.h \
  /cygdrive/d/sdk4.0ad.1.3_jade_c_20121207_16bit_new_new/inc/hld/hld_dev.h \
  /cygdrive/d/sdk4.0ad.1.3_jade_c_20121207_16bit_new_new/inc/bus/erom/erom.h \
  /cygdrive/d/sdk4.0ad.1.3_jade_c_20121207_16bit_new_new/inc/api/libstbinfo/stb_info.h \
  /cygdrive/d/sdk4.0ad.1.3_jade_c_20121207_16bit_new_new/inc/api/libstbinfo/stb_info_data.h \
  /cygdrive/d/sdk4.0ad.1.3_jade_c_20121207_16bit_new_new/inc/api/libloader/ld_boot_info.h \
  /cygdrive/d/sdk4.0ad.1.3_jade_c_20121207_16bit_new_new/inc/api/libpub/lib_frontend.h \
  /cygdrive/d/sdk4.0ad.1.3_jade_c_20121207_16bit_new_new/inc/api/libtsi/p_search.h \
  /cygdrive/d/sdk4.0ad.1.3_jade_c_20121207_16bit_new_new/inc/api/libsi/si_config.h \
  /cygdrive/d/sdk4.0ad.1.3_jade_c_20121207_16bit_new_new/inc/api/libtsi/sec_pmt.h \
  /cygdrive/d/sdk4.0ad.1.3_jade_c_20121207_16bit_new_new/inc/api/libtsi/sec_sdt.h \
  /cygdrive/d/sdk4.0ad.1.3_jade_c_20121207_16bit_new_new/inc/api/libtsi/db_3l.h \
  /cygdrive/d/sdk4.0ad.1.3_jade_c_20121207_16bit_new_new/inc/api/libdb/db_return_value.h \
  /cygdrive/d/sdk4.0ad.1.3_jade_c_20121207_16bit_new_new/inc/api/libdb/db_config.h \
  /cygdrive/d/sdk4.0ad.1.3_jade_c_20121207_16bit_new_new/inc/api/libdb/db_node_c.h \
  /cygdrive/d/sdk4.0ad.1.3_jade_c_20121207_16bit_new_new/inc/api/libdb/db_interface.h
boot_osd_rsc.o: boot_osd_rsc.c \
  /cygdrive/d/sdk4.0ad.1.3_jade_c_20121207_16bit_new_new/prj/app/demo/dvbc/jade_demo_new_16bit/upg_loader/mcf/release/include/sys_config.h \
  /cygdrive/d/sdk4.0ad.1.3_jade_c_20121207_16bit_new_new/inc/sys_define.h \
  /cygdrive/d/sdk4.0ad.1.3_jade_c_20121207_16bit_new_new/prj/app/demo/dvbc/jade_demo_new_16bit/upg_loader/mcf/release/include/common_config.h \
  /cygdrive/d/sdk4.0ad.1.3_jade_c_20121207_16bit_new_new/inc/sys_parameters.h \
  /cygdrive/d/sdk4.0ad.1.3_jade_c_20121207_16bit_new_new/inc/api/libc/printf.h \
  /cygdrive/d/sdk4.0ad.1.3_jade_c_20121207_16bit_new_new/inc/osal/osal.h \
  /cygdrive/d/sdk4.0ad.1.3_jade_c_20121207_16bit_new_new/inc/retcode.h \
  /cygdrive/d/sdk4.0ad.1.3_jade_c_20121207_16bit_new_new/inc/types.h \
  /cygdrive/d/sdk4.0ad.1.3_jade_c_20121207_16bit_new_new/inc/basic_types.h \
  /cygdrive/d/sdk4.0ad.1.3_jade_c_20121207_16bit_new_new/inc/os/tds2/itron.h \
  /cygdrive/d/sdk4.0ad.1.3_jade_c_20121207_16bit_new_new/inc/os/tds2/alitypes.h \
  /cygdrive/d/sdk4.0ad.1.3_jade_c_20121207_16bit_new_new/inc/os/tds2/platform.h \
  /cygdrive/d/sdk4.0ad.1.3_jade_c_20121207_16bit_new_new/inc/os/tds2/config.h \
  /cygdrive/d/sdk4.0ad.1.3_jade_c_20121207_16bit_new_new/inc/osal/osal_task.h \
  /cygdrive/d/sdk4.0ad.1.3_jade_c_20121207_16bit_new_new/inc/osal/osal_int.h \
  /cygdrive/d/sdk4.0ad.1.3_jade_c_20121207_16bit_new_new/inc/osal/osal_timer.h \
  /cygdrive/d/sdk4.0ad.1.3_jade_c_20121207_16bit_new_new/inc/osal/osal_msgq.h \
  /cygdrive/d/sdk4.0ad.1.3_jade_c_20121207_16bit_new_new/inc/osal/osal_sema.h \
  /cygdrive/d/sdk4.0ad.1.3_jade_c_20121207_16bit_new_new/inc/osal/osal_flag.h \
  /cygdrive/d/sdk4.0ad.1.3_jade_c_20121207_16bit_new_new/inc/osal/osal_mm.h \
  /cygdrive/d/sdk4.0ad.1.3_jade_c_20121207_16bit_new_new/inc/api/libc/alloc.h \
  /cygdrive/d/sdk4.0ad.1.3_jade_c_20121207_16bit_new_new/inc/osal/osal_cache.h \
  /cygdrive/d/sdk4.0ad.1.3_jade_c_20121207_16bit_new_new/inc/osal/osal_mutex.h \
  /cygdrive/d/sdk4.0ad.1.3_jade_c_20121207_16bit_new_new/inc/hal/machine.h \
  /cygdrive/D/study/v2.1/ALi_IDE_2.1_compile_tools/compilers/gcc3.0.4/lib/gcc-lib/mips-t2-elf/3.0.4/include/stdarg.h \
  /cygdrive/d/sdk4.0ad.1.3_jade_c_20121207_16bit_new_new/inc/api/libc/string.h \
  /cygdrive/d/sdk4.0ad.1.3_jade_c_20121207_16bit_new_new/inc/hld/pan/pan_dev.h \
  /cygdrive/d/sdk4.0ad.1.3_jade_c_20121207_16bit_new_new/inc/hld/hld_dev.h \
  /cygdrive/d/sdk4.0ad.1.3_jade_c_20121207_16bit_new_new/inc/hld/osd/osddrv_dev.h \
  /cygdrive/d/sdk4.0ad.1.3_jade_c_20121207_16bit_new_new/inc/hld/osd/osddrv.h \
  /cygdrive/d/sdk4.0ad.1.3_jade_c_20121207_16bit_new_new/inc/api/libloader/ld_osd_lib.h \
  /cygdrive/d/sdk4.0ad.1.3_jade_c_20121207_16bit_new_new/inc/mediatypes.h \
  /cygdrive/d/sdk4.0ad.1.3_jade_c_20121207_16bit_new_new/inc/api/libchar/lib_char.h \
  /cygdrive/d/sdk4.0ad.1.3_jade_c_20121207_16bit_new_new/inc/api/libchunk/chunk.h \
  /cygdrive/d/sdk4.0ad.1.3_jade_c_20121207_16bit_new_new/inc/hld/sto/sto.h \
  /cygdrive/d/sdk4.0ad.1.3_jade_c_20121207_16bit_new_new/inc/hld/sto/sto_dev.h \
  /cygdrive/d/sdk4.0ad.1.3_jade_c_20121207_16bit_new_new/inc/bus/erom/erom.h \
  boot_osd_rsc.h dev_handle.h upg_common.h \
  ../copper_common/boot_system_data.h ../copper_common/stb_data.h \
  /cygdrive/D/study/v2.1/ALi_IDE_2.1_compile_tools/compilers/gcc3.0.4/mips-t2-elf/include/math.h \
  /cygdrive/D/study/v2.1/ALi_IDE_2.1_compile_tools/compilers/gcc3.0.4/mips-t2-elf/include/sys/reent.h \
  /cygdrive/D/study/v2.1/ALi_IDE_2.1_compile_tools/compilers/gcc3.0.4/mips-t2-elf/include/_ansi.h \
  /cygdrive/D/study/v2.1/ALi_IDE_2.1_compile_tools/compilers/gcc3.0.4/mips-t2-elf/include/sys/config.h \
  /cygdrive/D/study/v2.1/ALi_IDE_2.1_compile_tools/compilers/gcc3.0.4/mips-t2-elf/include/time.h \
  /cygdrive/D/study/v2.1/ALi_IDE_2.1_compile_tools/compilers/gcc3.0.4/mips-t2-elf/include/machine/time.h \
  /cygdrive/D/study/v2.1/ALi_IDE_2.1_compile_tools/compilers/gcc3.0.4/lib/gcc-lib/mips-t2-elf/3.0.4/include/stddef.h \
  /cygdrive/D/study/v2.1/ALi_IDE_2.1_compile_tools/compilers/gcc3.0.4/mips-t2-elf/include/sys/types.h \
  /cygdrive/D/study/v2.1/ALi_IDE_2.1_compile_tools/compilers/gcc3.0.4/mips-t2-elf/include/machine/types.h \
  /cygdrive/D/study/v2.1/ALi_IDE_2.1_compile_tools/compilers/gcc3.0.4/mips-t2-elf/include/sys/features.h \
  /cygdrive/D/study/v2.1/ALi_IDE_2.1_compile_tools/compilers/gcc3.0.4/mips-t2-elf/include/machine/ieeefp.h \
  /cygdrive/d/sdk4.0ad.1.3_jade_c_20121207_16bit_new_new/inc/api/libstbinfo/stb_info.h \
  /cygdrive/d/sdk4.0ad.1.3_jade_c_20121207_16bit_new_new/inc/api/libstbinfo/stb_info_data.h \
  /cygdrive/d/sdk4.0ad.1.3_jade_c_20121207_16bit_new_new/inc/api/libloader/ld_boot_info.h \
  /cygdrive/d/sdk4.0ad.1.3_jade_c_20121207_16bit_new_new/inc/api/libpub/lib_frontend.h \
  /cygdrive/d/sdk4.0ad.1.3_jade_c_20121207_16bit_new_new/inc/api/libtsi/p_search.h \
  /cygdrive/d/sdk4.0ad.1.3_jade_c_20121207_16bit_new_new/inc/api/libsi/si_config.h \
  /cygdrive/d/sdk4.0ad.1.3_jade_c_20121207_16bit_new_new/inc/api/libtsi/sec_pmt.h \
  /cygdrive/d/sdk4.0ad.1.3_jade_c_20121207_16bit_new_new/inc/api/libtsi/sec_sdt.h \
  /cygdrive/d/sdk4.0ad.1.3_jade_c_20121207_16bit_new_new/inc/api/libtsi/db_3l.h \
  /cygdrive/d/sdk4.0ad.1.3_jade_c_20121207_16bit_new_new/inc/api/libdb/db_return_value.h \
  /cygdrive/d/sdk4.0ad.1.3_jade_c_20121207_16bit_new_new/inc/api/libdb/db_config.h \
  /cygdrive/d/sdk4.0ad.1.3_jade_c_20121207_16bit_new_new/inc/api/libdb/db_node_c.h \
  /cygdrive/d/sdk4.0ad.1.3_jade_c_20121207_16bit_new_new/inc/api/libdb/db_interface.h \
  resource/font/fonts_bak_array.h resource/string_tab/strings_array.h \
  resource/string_tab/str_English.h resource/string_tab/str_s_chinese.h
key.o: key.c \
  /cygdrive/d/sdk4.0ad.1.3_jade_c_20121207_16bit_new_new/prj/app/demo/dvbc/jade_demo_new_16bit/upg_loader/mcf/release/include/sys_config.h \
  /cygdrive/d/sdk4.0ad.1.3_jade_c_20121207_16bit_new_new/inc/sys_define.h \
  /cygdrive/d/sdk4.0ad.1.3_jade_c_20121207_16bit_new_new/prj/app/demo/dvbc/jade_demo_new_16bit/upg_loader/mcf/release/include/common_config.h \
  /cygdrive/d/sdk4.0ad.1.3_jade_c_20121207_16bit_new_new/inc/retcode.h \
  /cygdrive/d/sdk4.0ad.1.3_jade_c_20121207_16bit_new_new/inc/types.h \
  /cygdrive/d/sdk4.0ad.1.3_jade_c_20121207_16bit_new_new/inc/basic_types.h \
  /cygdrive/d/sdk4.0ad.1.3_jade_c_20121207_16bit_new_new/inc/osal/osal.h \
  /cygdrive/d/sdk4.0ad.1.3_jade_c_20121207_16bit_new_new/inc/os/tds2/itron.h \
  /cygdrive/d/sdk4.0ad.1.3_jade_c_20121207_16bit_new_new/inc/os/tds2/alitypes.h \
  /cygdrive/d/sdk4.0ad.1.3_jade_c_20121207_16bit_new_new/inc/os/tds2/platform.h \
  /cygdrive/d/sdk4.0ad.1.3_jade_c_20121207_16bit_new_new/inc/os/tds2/config.h \
  /cygdrive/d/sdk4.0ad.1.3_jade_c_20121207_16bit_new_new/inc/api/libc/printf.h \
  /cygdrive/d/sdk4.0ad.1.3_jade_c_20121207_16bit_new_new/inc/hal/machine.h \
  /cygdrive/D/study/v2.1/ALi_IDE_2.1_compile_tools/compilers/gcc3.0.4/lib/gcc-lib/mips-t2-elf/3.0.4/include/stdarg.h \
  /cygdrive/d/sdk4.0ad.1.3_jade_c_20121207_16bit_new_new/inc/osal/osal_task.h \
  /cygdrive/d/sdk4.0ad.1.3_jade_c_20121207_16bit_new_new/inc/osal/osal_int.h \
  /cygdrive/d/sdk4.0ad.1.3_jade_c_20121207_16bit_new_new/inc/osal/osal_timer.h \
  /cygdrive/d/sdk4.0ad.1.3_jade_c_20121207_16bit_new_new/inc/osal/osal_msgq.h \
  /cygdrive/d/sdk4.0ad.1.3_jade_c_20121207_16bit_new_new/inc/osal/osal_sema.h \
  /cygdrive/d/sdk4.0ad.1.3_jade_c_20121207_16bit_new_new/inc/osal/osal_flag.h \
  /cygdrive/d/sdk4.0ad.1.3_jade_c_20121207_16bit_new_new/inc/osal/osal_mm.h \
  /cygdrive/d/sdk4.0ad.1.3_jade_c_20121207_16bit_new_new/inc/api/libc/alloc.h \
  /cygdrive/d/sdk4.0ad.1.3_jade_c_20121207_16bit_new_new/inc/osal/osal_cache.h \
  /cygdrive/d/sdk4.0ad.1.3_jade_c_20121207_16bit_new_new/inc/osal/osal_mutex.h \
  /cygdrive/d/sdk4.0ad.1.3_jade_c_20121207_16bit_new_new/inc/api/libc/string.h \
  /cygdrive/d/sdk4.0ad.1.3_jade_c_20121207_16bit_new_new/inc/hld/pan/pan_dev.h \
  /cygdrive/d/sdk4.0ad.1.3_jade_c_20121207_16bit_new_new/inc/hld/hld_dev.h \
  /cygdrive/d/sdk4.0ad.1.3_jade_c_20121207_16bit_new_new/inc/hld/pan/pan.h \
  /cygdrive/d/sdk4.0ad.1.3_jade_c_20121207_16bit_new_new/inc/hld/pan/pan_key.h \
  /cygdrive/d/sdk4.0ad.1.3_jade_c_20121207_16bit_new_new/inc/api/libosd/osd_vkey.h \
  dev_handle.h upg_common.h key.h
boot_ota_input.o: boot_ota_input.c \
  /cygdrive/d/sdk4.0ad.1.3_jade_c_20121207_16bit_new_new/prj/app/demo/dvbc/jade_demo_new_16bit/upg_loader/mcf/release/include/sys_config.h \
  /cygdrive/d/sdk4.0ad.1.3_jade_c_20121207_16bit_new_new/inc/sys_define.h \
  /cygdrive/d/sdk4.0ad.1.3_jade_c_20121207_16bit_new_new/prj/app/demo/dvbc/jade_demo_new_16bit/upg_loader/mcf/release/include/common_config.h \
  /cygdrive/d/sdk4.0ad.1.3_jade_c_20121207_16bit_new_new/inc/sys_parameters.h \
  /cygdrive/d/sdk4.0ad.1.3_jade_c_20121207_16bit_new_new/inc/api/libc/printf.h \
  /cygdrive/d/sdk4.0ad.1.3_jade_c_20121207_16bit_new_new/inc/osal/osal.h \
  /cygdrive/d/sdk4.0ad.1.3_jade_c_20121207_16bit_new_new/inc/retcode.h \
  /cygdrive/d/sdk4.0ad.1.3_jade_c_20121207_16bit_new_new/inc/types.h \
  /cygdrive/d/sdk4.0ad.1.3_jade_c_20121207_16bit_new_new/inc/basic_types.h \
  /cygdrive/d/sdk4.0ad.1.3_jade_c_20121207_16bit_new_new/inc/os/tds2/itron.h \
  /cygdrive/d/sdk4.0ad.1.3_jade_c_20121207_16bit_new_new/inc/os/tds2/alitypes.h \
  /cygdrive/d/sdk4.0ad.1.3_jade_c_20121207_16bit_new_new/inc/os/tds2/platform.h \
  /cygdrive/d/sdk4.0ad.1.3_jade_c_20121207_16bit_new_new/inc/os/tds2/config.h \
  /cygdrive/d/sdk4.0ad.1.3_jade_c_20121207_16bit_new_new/inc/osal/osal_task.h \
  /cygdrive/d/sdk4.0ad.1.3_jade_c_20121207_16bit_new_new/inc/osal/osal_int.h \
  /cygdrive/d/sdk4.0ad.1.3_jade_c_20121207_16bit_new_new/inc/osal/osal_timer.h \
  /cygdrive/d/sdk4.0ad.1.3_jade_c_20121207_16bit_new_new/inc/osal/osal_msgq.h \
  /cygdrive/d/sdk4.0ad.1.3_jade_c_20121207_16bit_new_new/inc/osal/osal_sema.h \
  /cygdrive/d/sdk4.0ad.1.3_jade_c_20121207_16bit_new_new/inc/osal/osal_flag.h \
  /cygdrive/d/sdk4.0ad.1.3_jade_c_20121207_16bit_new_new/inc/osal/osal_mm.h \
  /cygdrive/d/sdk4.0ad.1.3_jade_c_20121207_16bit_new_new/inc/api/libc/alloc.h \
  /cygdrive/d/sdk4.0ad.1.3_jade_c_20121207_16bit_new_new/inc/osal/osal_cache.h \
  /cygdrive/d/sdk4.0ad.1.3_jade_c_20121207_16bit_new_new/inc/osal/osal_mutex.h \
  /cygdrive/d/sdk4.0ad.1.3_jade_c_20121207_16bit_new_new/inc/hal/machine.h \
  /cygdrive/D/study/v2.1/ALi_IDE_2.1_compile_tools/compilers/gcc3.0.4/lib/gcc-lib/mips-t2-elf/3.0.4/include/stdarg.h \
  /cygdrive/d/sdk4.0ad.1.3_jade_c_20121207_16bit_new_new/inc/api/libc/string.h \
  /cygdrive/d/sdk4.0ad.1.3_jade_c_20121207_16bit_new_new/inc/api/libchunk/chunk.h \
  /cygdrive/d/sdk4.0ad.1.3_jade_c_20121207_16bit_new_new/inc/hld/sto/sto.h \
  /cygdrive/d/sdk4.0ad.1.3_jade_c_20121207_16bit_new_new/inc/hld/sto/sto_dev.h \
  /cygdrive/d/sdk4.0ad.1.3_jade_c_20121207_16bit_new_new/inc/hld/hld_dev.h \
  /cygdrive/d/sdk4.0ad.1.3_jade_c_20121207_16bit_new_new/inc/bus/erom/erom.h \
  /cygdrive/d/sdk4.0ad.1.3_jade_c_20121207_16bit_new_new/inc/bus/sci/sci.h \
  /cygdrive/d/sdk4.0ad.1.3_jade_c_20121207_16bit_new_new/inc/bus/flash/flash.h \
  /cygdrive/d/sdk4.0ad.1.3_jade_c_20121207_16bit_new_new/inc/hal/hal_gpio.h \
  /cygdrive/d/sdk4.0ad.1.3_jade_c_20121207_16bit_new_new/inc/hal/hal_mem.h \
  /cygdrive/d/sdk4.0ad.1.3_jade_c_20121207_16bit_new_new/inc/hld/pan/pan_dev.h \
  /cygdrive/d/sdk4.0ad.1.3_jade_c_20121207_16bit_new_new/inc/hld/pan/pan.h \
  /cygdrive/d/sdk4.0ad.1.3_jade_c_20121207_16bit_new_new/inc/api/libota/lib_ota.h \
  /cygdrive/d/sdk4.0ad.1.3_jade_c_20121207_16bit_new_new/inc/api/libtsi/si_types.h \
  /cygdrive/d/sdk4.0ad.1.3_jade_c_20121207_16bit_new_new/inc/hld/osd/osddrv.h \
  /cygdrive/d/sdk4.0ad.1.3_jade_c_20121207_16bit_new_new/inc/hld/osd/osddrv_dev.h \
  /cygdrive/d/sdk4.0ad.1.3_jade_c_20121207_16bit_new_new/inc/api/libloader/ld_osd_lib.h \
  /cygdrive/d/sdk4.0ad.1.3_jade_c_20121207_16bit_new_new/inc/mediatypes.h \
  /cygdrive/d/sdk4.0ad.1.3_jade_c_20121207_16bit_new_new/inc/api/libchar/lib_char.h \
  /cygdrive/d/sdk4.0ad.1.3_jade_c_20121207_16bit_new_new/inc/api/libloader/ld_boot_info.h \
  /cygdrive/d/sdk4.0ad.1.3_jade_c_20121207_16bit_new_new/inc/api/libpub/lib_frontend.h \
  /cygdrive/d/sdk4.0ad.1.3_jade_c_20121207_16bit_new_new/inc/api/libtsi/p_search.h \
  /cygdrive/d/sdk4.0ad.1.3_jade_c_20121207_16bit_new_new/inc/api/libsi/si_config.h \
  /cygdrive/d/sdk4.0ad.1.3_jade_c_20121207_16bit_new_new/inc/api/libtsi/sec_pmt.h \
  /cygdrive/d/sdk4.0ad.1.3_jade_c_20121207_16bit_new_new/inc/api/libtsi/sec_sdt.h \
  /cygdrive/d/sdk4.0ad.1.3_jade_c_20121207_16bit_new_new/inc/api/libtsi/db_3l.h \
  /cygdrive/d/sdk4.0ad.1.3_jade_c_20121207_16bit_new_new/inc/api/libdb/db_return_value.h \
  /cygdrive/d/sdk4.0ad.1.3_jade_c_20121207_16bit_new_new/inc/api/libdb/db_config.h \
  /cygdrive/d/sdk4.0ad.1.3_jade_c_20121207_16bit_new_new/inc/api/libdb/db_node_c.h \
  /cygdrive/d/sdk4.0ad.1.3_jade_c_20121207_16bit_new_new/inc/api/libdb/db_interface.h \
  /cygdrive/d/sdk4.0ad.1.3_jade_c_20121207_16bit_new_new/inc/api/libosd/osd_vkey.h \
  /cygdrive/d/sdk4.0ad.1.3_jade_c_20121207_16bit_new_new/inc/api/libstbinfo/stb_info_data.h \
  dev_handle.h upg_common.h ../copper_common/boot_system_data.h \
  ../copper_common/stb_data.h \
  /cygdrive/D/study/v2.1/ALi_IDE_2.1_compile_tools/compilers/gcc3.0.4/mips-t2-elf/include/math.h \
  /cygdrive/D/study/v2.1/ALi_IDE_2.1_compile_tools/compilers/gcc3.0.4/mips-t2-elf/include/sys/reent.h \
  /cygdrive/D/study/v2.1/ALi_IDE_2.1_compile_tools/compilers/gcc3.0.4/mips-t2-elf/include/_ansi.h \
  /cygdrive/D/study/v2.1/ALi_IDE_2.1_compile_tools/compilers/gcc3.0.4/mips-t2-elf/include/sys/config.h \
  /cygdrive/D/study/v2.1/ALi_IDE_2.1_compile_tools/compilers/gcc3.0.4/mips-t2-elf/include/time.h \
  /cygdrive/D/study/v2.1/ALi_IDE_2.1_compile_tools/compilers/gcc3.0.4/mips-t2-elf/include/machine/time.h \
  /cygdrive/D/study/v2.1/ALi_IDE_2.1_compile_tools/compilers/gcc3.0.4/lib/gcc-lib/mips-t2-elf/3.0.4/include/stddef.h \
  /cygdrive/D/study/v2.1/ALi_IDE_2.1_compile_tools/compilers/gcc3.0.4/mips-t2-elf/include/sys/types.h \
  /cygdrive/D/study/v2.1/ALi_IDE_2.1_compile_tools/compilers/gcc3.0.4/mips-t2-elf/include/machine/types.h \
  /cygdrive/D/study/v2.1/ALi_IDE_2.1_compile_tools/compilers/gcc3.0.4/mips-t2-elf/include/sys/features.h \
  /cygdrive/D/study/v2.1/ALi_IDE_2.1_compile_tools/compilers/gcc3.0.4/mips-t2-elf/include/machine/ieeefp.h \
  /cygdrive/d/sdk4.0ad.1.3_jade_c_20121207_16bit_new_new/inc/api/libstbinfo/stb_info.h \
  key.h string.id
upgloader.o: upgloader.c \
  /cygdrive/d/sdk4.0ad.1.3_jade_c_20121207_16bit_new_new/prj/app/demo/dvbc/jade_demo_new_16bit/upg_loader/mcf/release/include/sys_config.h \
  /cygdrive/d/sdk4.0ad.1.3_jade_c_20121207_16bit_new_new/inc/sys_define.h \
  /cygdrive/d/sdk4.0ad.1.3_jade_c_20121207_16bit_new_new/prj/app/demo/dvbc/jade_demo_new_16bit/upg_loader/mcf/release/include/common_config.h \
  /cygdrive/d/sdk4.0ad.1.3_jade_c_20121207_16bit_new_new/inc/sys_parameters.h \
  /cygdrive/d/sdk4.0ad.1.3_jade_c_20121207_16bit_new_new/inc/api/libc/printf.h \
  /cygdrive/d/sdk4.0ad.1.3_jade_c_20121207_16bit_new_new/inc/osal/osal.h \
  /cygdrive/d/sdk4.0ad.1.3_jade_c_20121207_16bit_new_new/inc/retcode.h \
  /cygdrive/d/sdk4.0ad.1.3_jade_c_20121207_16bit_new_new/inc/types.h \
  /cygdrive/d/sdk4.0ad.1.3_jade_c_20121207_16bit_new_new/inc/basic_types.h \
  /cygdrive/d/sdk4.0ad.1.3_jade_c_20121207_16bit_new_new/inc/os/tds2/itron.h \
  /cygdrive/d/sdk4.0ad.1.3_jade_c_20121207_16bit_new_new/inc/os/tds2/alitypes.h \
  /cygdrive/d/sdk4.0ad.1.3_jade_c_20121207_16bit_new_new/inc/os/tds2/platform.h \
  /cygdrive/d/sdk4.0ad.1.3_jade_c_20121207_16bit_new_new/inc/os/tds2/config.h \
  /cygdrive/d/sdk4.0ad.1.3_jade_c_20121207_16bit_new_new/inc/osal/osal_task.h \
  /cygdrive/d/sdk4.0ad.1.3_jade_c_20121207_16bit_new_new/inc/osal/osal_int.h \
  /cygdrive/d/sdk4.0ad.1.3_jade_c_20121207_16bit_new_new/inc/osal/osal_timer.h \
  /cygdrive/d/sdk4.0ad.1.3_jade_c_20121207_16bit_new_new/inc/osal/osal_msgq.h \
  /cygdrive/d/sdk4.0ad.1.3_jade_c_20121207_16bit_new_new/inc/osal/osal_sema.h \
  /cygdrive/d/sdk4.0ad.1.3_jade_c_20121207_16bit_new_new/inc/osal/osal_flag.h \
  /cygdrive/d/sdk4.0ad.1.3_jade_c_20121207_16bit_new_new/inc/osal/osal_mm.h \
  /cygdrive/d/sdk4.0ad.1.3_jade_c_20121207_16bit_new_new/inc/api/libc/alloc.h \
  /cygdrive/d/sdk4.0ad.1.3_jade_c_20121207_16bit_new_new/inc/osal/osal_cache.h \
  /cygdrive/d/sdk4.0ad.1.3_jade_c_20121207_16bit_new_new/inc/osal/osal_mutex.h \
  /cygdrive/d/sdk4.0ad.1.3_jade_c_20121207_16bit_new_new/inc/hal/machine.h \
  /cygdrive/D/study/v2.1/ALi_IDE_2.1_compile_tools/compilers/gcc3.0.4/lib/gcc-lib/mips-t2-elf/3.0.4/include/stdarg.h \
  /cygdrive/d/sdk4.0ad.1.3_jade_c_20121207_16bit_new_new/inc/api/libc/string.h \
  /cygdrive/d/sdk4.0ad.1.3_jade_c_20121207_16bit_new_new/inc/api/libchunk/chunk.h \
  /cygdrive/d/sdk4.0ad.1.3_jade_c_20121207_16bit_new_new/inc/hld/sto/sto.h \
  /cygdrive/d/sdk4.0ad.1.3_jade_c_20121207_16bit_new_new/inc/hld/sto/sto_dev.h \
  /cygdrive/d/sdk4.0ad.1.3_jade_c_20121207_16bit_new_new/inc/hld/hld_dev.h \
  /cygdrive/d/sdk4.0ad.1.3_jade_c_20121207_16bit_new_new/inc/bus/erom/erom.h \
  /cygdrive/d/sdk4.0ad.1.3_jade_c_20121207_16bit_new_new/inc/bus/sci/sci.h \
  /cygdrive/d/sdk4.0ad.1.3_jade_c_20121207_16bit_new_new/inc/bus/flash/flash.h \
  /cygdrive/d/sdk4.0ad.1.3_jade_c_20121207_16bit_new_new/inc/hal/hal_gpio.h \
  /cygdrive/d/sdk4.0ad.1.3_jade_c_20121207_16bit_new_new/inc/hal/hal_mem.h \
  /cygdrive/d/sdk4.0ad.1.3_jade_c_20121207_16bit_new_new/inc/hld/pan/pan_dev.h \
  /cygdrive/d/sdk4.0ad.1.3_jade_c_20121207_16bit_new_new/inc/hld/pan/pan.h \
  /cygdrive/d/sdk4.0ad.1.3_jade_c_20121207_16bit_new_new/inc/hld/nim/nim_tuner.h \
  /cygdrive/d/sdk4.0ad.1.3_jade_c_20121207_16bit_new_new/inc/hld/dmx/dmx.h \
  /cygdrive/d/sdk4.0ad.1.3_jade_c_20121207_16bit_new_new/inc/hld/dmx/dmx_dev.h \
  /cygdrive/d/sdk4.0ad.1.3_jade_c_20121207_16bit_new_new/inc/mediatypes.h \
  /cygdrive/d/sdk4.0ad.1.3_jade_c_20121207_16bit_new_new/inc/bus/dma/dma.h \
  /cygdrive/d/sdk4.0ad.1.3_jade_c_20121207_16bit_new_new/inc/bus/i2c/i2c.h \
  /cygdrive/d/sdk4.0ad.1.3_jade_c_20121207_16bit_new_new/inc/bus/tsi/tsi.h \
  /cygdrive/d/sdk4.0ad.1.3_jade_c_20121207_16bit_new_new/inc/api/libota/lib_ota.h \
  /cygdrive/d/sdk4.0ad.1.3_jade_c_20121207_16bit_new_new/inc/api/libtsi/si_types.h \
  /cygdrive/d/sdk4.0ad.1.3_jade_c_20121207_16bit_new_new/inc/hld/scart/scart.h \
  /cygdrive/d/sdk4.0ad.1.3_jade_c_20121207_16bit_new_new/inc/hld/osd/osddrv.h \
  /cygdrive/d/sdk4.0ad.1.3_jade_c_20121207_16bit_new_new/inc/hld/osd/osddrv_dev.h \
  /cygdrive/d/sdk4.0ad.1.3_jade_c_20121207_16bit_new_new/inc/api/libloader/ld_osd_lib.h \
  /cygdrive/d/sdk4.0ad.1.3_jade_c_20121207_16bit_new_new/inc/api/libchar/lib_char.h \
  /cygdrive/d/sdk4.0ad.1.3_jade_c_20121207_16bit_new_new/inc/api/libloader/ld_boot_info.h \
  /cygdrive/d/sdk4.0ad.1.3_jade_c_20121207_16bit_new_new/inc/api/libpub/lib_frontend.h \
  /cygdrive/d/sdk4.0ad.1.3_jade_c_20121207_16bit_new_new/inc/api/libtsi/p_search.h \
  /cygdrive/d/sdk4.0ad.1.3_jade_c_20121207_16bit_new_new/inc/api/libsi/si_config.h \
  /cygdrive/d/sdk4.0ad.1.3_jade_c_20121207_16bit_new_new/inc/api/libtsi/sec_pmt.h \
  /cygdrive/d/sdk4.0ad.1.3_jade_c_20121207_16bit_new_new/inc/api/libtsi/sec_sdt.h \
  /cygdrive/d/sdk4.0ad.1.3_jade_c_20121207_16bit_new_new/inc/api/libtsi/db_3l.h \
  /cygdrive/d/sdk4.0ad.1.3_jade_c_20121207_16bit_new_new/inc/api/libdb/db_return_value.h \
  /cygdrive/d/sdk4.0ad.1.3_jade_c_20121207_16bit_new_new/inc/api/libdb/db_config.h \
  /cygdrive/d/sdk4.0ad.1.3_jade_c_20121207_16bit_new_new/inc/api/libdb/db_node_c.h \
  /cygdrive/d/sdk4.0ad.1.3_jade_c_20121207_16bit_new_new/inc/api/libdb/db_interface.h \
  /cygdrive/d/sdk4.0ad.1.3_jade_c_20121207_16bit_new_new/inc/api/libosd/osd_vkey.h \
  /cygdrive/d/sdk4.0ad.1.3_jade_c_20121207_16bit_new_new/inc/hld/dis/vpo.h \
  /cygdrive/d/sdk4.0ad.1.3_jade_c_20121207_16bit_new_new/inc/bus/hdmi/m36/hdmi_api.h \
  /cygdrive/d/sdk4.0ad.1.3_jade_c_20121207_16bit_new_new/inc/hld/ge/ge.h \
  /cygdrive/d/sdk4.0ad.1.3_jade_c_20121207_16bit_new_new/inc/hld/ge/ge_old.h \
  /cygdrive/d/sdk4.0ad.1.3_jade_c_20121207_16bit_new_new/inc/hld/ge/ge_new.h \
  /cygdrive/d/sdk4.0ad.1.3_jade_c_20121207_16bit_new_new/inc/hld/ge/gma_new.h \
  /cygdrive/d/sdk4.0ad.1.3_jade_c_20121207_16bit_new_new/inc/hld/decv/decv.h \
  /cygdrive/d/sdk4.0ad.1.3_jade_c_20121207_16bit_new_new/inc/hld/decv/decv_avc.h \
  /cygdrive/d/sdk4.0ad.1.3_jade_c_20121207_16bit_new_new/inc/hld/nim/nim_dev.h \
  /cygdrive/d/sdk4.0ad.1.3_jade_c_20121207_16bit_new_new/inc/api/libpvr/lib_pvr.h \
  /cygdrive/d/sdk4.0ad.1.3_jade_c_20121207_16bit_new_new/inc/api/libstbinfo/stb_info_data.h \
  ../copper_common/boot_system_data.h ../copper_common/stb_data.h \
  /cygdrive/D/study/v2.1/ALi_IDE_2.1_compile_tools/compilers/gcc3.0.4/mips-t2-elf/include/math.h \
  /cygdrive/D/study/v2.1/ALi_IDE_2.1_compile_tools/compilers/gcc3.0.4/mips-t2-elf/include/sys/reent.h \
  /cygdrive/D/study/v2.1/ALi_IDE_2.1_compile_tools/compilers/gcc3.0.4/mips-t2-elf/include/_ansi.h \
  /cygdrive/D/study/v2.1/ALi_IDE_2.1_compile_tools/compilers/gcc3.0.4/mips-t2-elf/include/sys/config.h \
  /cygdrive/D/study/v2.1/ALi_IDE_2.1_compile_tools/compilers/gcc3.0.4/mips-t2-elf/include/time.h \
  /cygdrive/D/study/v2.1/ALi_IDE_2.1_compile_tools/compilers/gcc3.0.4/mips-t2-elf/include/machine/time.h \
  /cygdrive/D/study/v2.1/ALi_IDE_2.1_compile_tools/compilers/gcc3.0.4/lib/gcc-lib/mips-t2-elf/3.0.4/include/stddef.h \
  /cygdrive/D/study/v2.1/ALi_IDE_2.1_compile_tools/compilers/gcc3.0.4/mips-t2-elf/include/sys/types.h \
  /cygdrive/D/study/v2.1/ALi_IDE_2.1_compile_tools/compilers/gcc3.0.4/mips-t2-elf/include/machine/types.h \
  /cygdrive/D/study/v2.1/ALi_IDE_2.1_compile_tools/compilers/gcc3.0.4/mips-t2-elf/include/sys/features.h \
  /cygdrive/D/study/v2.1/ALi_IDE_2.1_compile_tools/compilers/gcc3.0.4/mips-t2-elf/include/machine/ieeefp.h \
  /cygdrive/d/sdk4.0ad.1.3_jade_c_20121207_16bit_new_new/inc/api/libstbinfo/stb_info.h \
  resource/pallette/palettes_array.h resource/pallette/palette1.h key.h \
  boot_osd_rsc.h dev_handle.h upg_common.h \
  /cygdrive/d/sdk4.0ad.1.3_jade_c_20121207_16bit_new_new/inc/bus/dog/dog.h
upg_common.o: upg_common.c \
  /cygdrive/d/sdk4.0ad.1.3_jade_c_20121207_16bit_new_new/prj/app/demo/dvbc/jade_demo_new_16bit/upg_loader/mcf/release/include/sys_config.h \
  /cygdrive/d/sdk4.0ad.1.3_jade_c_20121207_16bit_new_new/inc/sys_define.h \
  /cygdrive/d/sdk4.0ad.1.3_jade_c_20121207_16bit_new_new/prj/app/demo/dvbc/jade_demo_new_16bit/upg_loader/mcf/release/include/common_config.h \
  /cygdrive/d/sdk4.0ad.1.3_jade_c_20121207_16bit_new_new/inc/sys_parameters.h \
  /cygdrive/d/sdk4.0ad.1.3_jade_c_20121207_16bit_new_new/inc/api/libc/printf.h \
  /cygdrive/d/sdk4.0ad.1.3_jade_c_20121207_16bit_new_new/inc/osal/osal.h \
  /cygdrive/d/sdk4.0ad.1.3_jade_c_20121207_16bit_new_new/inc/retcode.h \
  /cygdrive/d/sdk4.0ad.1.3_jade_c_20121207_16bit_new_new/inc/types.h \
  /cygdrive/d/sdk4.0ad.1.3_jade_c_20121207_16bit_new_new/inc/basic_types.h \
  /cygdrive/d/sdk4.0ad.1.3_jade_c_20121207_16bit_new_new/inc/os/tds2/itron.h \
  /cygdrive/d/sdk4.0ad.1.3_jade_c_20121207_16bit_new_new/inc/os/tds2/alitypes.h \
  /cygdrive/d/sdk4.0ad.1.3_jade_c_20121207_16bit_new_new/inc/os/tds2/platform.h \
  /cygdrive/d/sdk4.0ad.1.3_jade_c_20121207_16bit_new_new/inc/os/tds2/config.h \
  /cygdrive/d/sdk4.0ad.1.3_jade_c_20121207_16bit_new_new/inc/osal/osal_task.h \
  /cygdrive/d/sdk4.0ad.1.3_jade_c_20121207_16bit_new_new/inc/osal/osal_int.h \
  /cygdrive/d/sdk4.0ad.1.3_jade_c_20121207_16bit_new_new/inc/osal/osal_timer.h \
  /cygdrive/d/sdk4.0ad.1.3_jade_c_20121207_16bit_new_new/inc/osal/osal_msgq.h \
  /cygdrive/d/sdk4.0ad.1.3_jade_c_20121207_16bit_new_new/inc/osal/osal_sema.h \
  /cygdrive/d/sdk4.0ad.1.3_jade_c_20121207_16bit_new_new/inc/osal/osal_flag.h \
  /cygdrive/d/sdk4.0ad.1.3_jade_c_20121207_16bit_new_new/inc/osal/osal_mm.h \
  /cygdrive/d/sdk4.0ad.1.3_jade_c_20121207_16bit_new_new/inc/api/libc/alloc.h \
  /cygdrive/d/sdk4.0ad.1.3_jade_c_20121207_16bit_new_new/inc/osal/osal_cache.h \
  /cygdrive/d/sdk4.0ad.1.3_jade_c_20121207_16bit_new_new/inc/osal/osal_mutex.h \
  /cygdrive/d/sdk4.0ad.1.3_jade_c_20121207_16bit_new_new/inc/hal/machine.h \
  /cygdrive/D/study/v2.1/ALi_IDE_2.1_compile_tools/compilers/gcc3.0.4/lib/gcc-lib/mips-t2-elf/3.0.4/include/stdarg.h \
  /cygdrive/d/sdk4.0ad.1.3_jade_c_20121207_16bit_new_new/inc/api/libc/string.h \
  /cygdrive/d/sdk4.0ad.1.3_jade_c_20121207_16bit_new_new/inc/api/libchunk/chunk.h \
  /cygdrive/d/sdk4.0ad.1.3_jade_c_20121207_16bit_new_new/inc/hld/sto/sto.h \
  /cygdrive/d/sdk4.0ad.1.3_jade_c_20121207_16bit_new_new/inc/hld/sto/sto_dev.h \
  /cygdrive/d/sdk4.0ad.1.3_jade_c_20121207_16bit_new_new/inc/hld/hld_dev.h \
  /cygdrive/d/sdk4.0ad.1.3_jade_c_20121207_16bit_new_new/inc/bus/erom/erom.h \
  /cygdrive/d/sdk4.0ad.1.3_jade_c_20121207_16bit_new_new/inc/bus/sci/sci.h \
  /cygdrive/d/sdk4.0ad.1.3_jade_c_20121207_16bit_new_new/inc/bus/flash/flash.h \
  /cygdrive/d/sdk4.0ad.1.3_jade_c_20121207_16bit_new_new/inc/hal/hal_gpio.h \
  /cygdrive/d/sdk4.0ad.1.3_jade_c_20121207_16bit_new_new/inc/hal/hal_mem.h \
  /cygdrive/d/sdk4.0ad.1.3_jade_c_20121207_16bit_new_new/inc/hld/pan/pan_dev.h \
  /cygdrive/d/sdk4.0ad.1.3_jade_c_20121207_16bit_new_new/inc/hld/pan/pan.h \
  /cygdrive/d/sdk4.0ad.1.3_jade_c_20121207_16bit_new_new/inc/api/libpub/lib_device_manage.h \
  /cygdrive/d/sdk4.0ad.1.3_jade_c_20121207_16bit_new_new/inc/api/libpub/lib_frontend.h \
  /cygdrive/d/sdk4.0ad.1.3_jade_c_20121207_16bit_new_new/inc/api/libtsi/p_search.h \
  /cygdrive/d/sdk4.0ad.1.3_jade_c_20121207_16bit_new_new/inc/api/libsi/si_config.h \
  /cygdrive/d/sdk4.0ad.1.3_jade_c_20121207_16bit_new_new/inc/api/libtsi/sec_pmt.h \
  /cygdrive/d/sdk4.0ad.1.3_jade_c_20121207_16bit_new_new/inc/api/libtsi/sec_sdt.h \
  /cygdrive/d/sdk4.0ad.1.3_jade_c_20121207_16bit_new_new/inc/api/libtsi/db_3l.h \
  /cygdrive/d/sdk4.0ad.1.3_jade_c_20121207_16bit_new_new/inc/api/libdb/db_return_value.h \
  /cygdrive/d/sdk4.0ad.1.3_jade_c_20121207_16bit_new_new/inc/api/libdb/db_config.h \
  /cygdrive/d/sdk4.0ad.1.3_jade_c_20121207_16bit_new_new/inc/api/libdb/db_node_c.h \
  /cygdrive/d/sdk4.0ad.1.3_jade_c_20121207_16bit_new_new/inc/api/libdb/db_interface.h \
  /cygdrive/d/sdk4.0ad.1.3_jade_c_20121207_16bit_new_new/inc/hld/osd/osddrv.h \
  /cygdrive/d/sdk4.0ad.1.3_jade_c_20121207_16bit_new_new/inc/hld/osd/osddrv_dev.h \
  /cygdrive/d/sdk4.0ad.1.3_jade_c_20121207_16bit_new_new/inc/api/libloader/ld_osd_lib.h \
  /cygdrive/d/sdk4.0ad.1.3_jade_c_20121207_16bit_new_new/inc/mediatypes.h \
  /cygdrive/d/sdk4.0ad.1.3_jade_c_20121207_16bit_new_new/inc/api/libchar/lib_char.h \
  /cygdrive/d/sdk4.0ad.1.3_jade_c_20121207_16bit_new_new/inc/api/libloader/ld_boot_info.h \
  /cygdrive/d/sdk4.0ad.1.3_jade_c_20121207_16bit_new_new/inc/api/libstbinfo/stb_info_data.h \
  /cygdrive/d/sdk4.0ad.1.3_jade_c_20121207_16bit_new_new/inc/api/libupg/lib_upg4.h \
  /cygdrive/d/sdk4.0ad.1.3_jade_c_20121207_16bit_new_new/inc/api/libtsi/si_types.h \
  dev_handle.h upg_common.h ../copper_common/boot_system_data.h \
  ../copper_common/stb_data.h \
  /cygdrive/D/study/v2.1/ALi_IDE_2.1_compile_tools/compilers/gcc3.0.4/mips-t2-elf/include/math.h \
  /cygdrive/D/study/v2.1/ALi_IDE_2.1_compile_tools/compilers/gcc3.0.4/mips-t2-elf/include/sys/reent.h \
  /cygdrive/D/study/v2.1/ALi_IDE_2.1_compile_tools/compilers/gcc3.0.4/mips-t2-elf/include/_ansi.h \
  /cygdrive/D/study/v2.1/ALi_IDE_2.1_compile_tools/compilers/gcc3.0.4/mips-t2-elf/include/sys/config.h \
  /cygdrive/D/study/v2.1/ALi_IDE_2.1_compile_tools/compilers/gcc3.0.4/mips-t2-elf/include/time.h \
  /cygdrive/D/study/v2.1/ALi_IDE_2.1_compile_tools/compilers/gcc3.0.4/mips-t2-elf/include/machine/time.h \
  /cygdrive/D/study/v2.1/ALi_IDE_2.1_compile_tools/compilers/gcc3.0.4/lib/gcc-lib/mips-t2-elf/3.0.4/include/stddef.h \
  /cygdrive/D/study/v2.1/ALi_IDE_2.1_compile_tools/compilers/gcc3.0.4/mips-t2-elf/include/sys/types.h \
  /cygdrive/D/study/v2.1/ALi_IDE_2.1_compile_tools/compilers/gcc3.0.4/mips-t2-elf/include/machine/types.h \
  /cygdrive/D/study/v2.1/ALi_IDE_2.1_compile_tools/compilers/gcc3.0.4/mips-t2-elf/include/sys/features.h \
  /cygdrive/D/study/v2.1/ALi_IDE_2.1_compile_tools/compilers/gcc3.0.4/mips-t2-elf/include/machine/ieeefp.h \
  /cygdrive/d/sdk4.0ad.1.3_jade_c_20121207_16bit_new_new/inc/api/libstbinfo/stb_info.h \
  string.id ../copper_common/stb_hwinfo.h
upg_ota.o: upg_ota.c \
  /cygdrive/d/sdk4.0ad.1.3_jade_c_20121207_16bit_new_new/prj/app/demo/dvbc/jade_demo_new_16bit/upg_loader/mcf/release/include/sys_config.h \
  /cygdrive/d/sdk4.0ad.1.3_jade_c_20121207_16bit_new_new/inc/sys_define.h \
  /cygdrive/d/sdk4.0ad.1.3_jade_c_20121207_16bit_new_new/prj/app/demo/dvbc/jade_demo_new_16bit/upg_loader/mcf/release/include/common_config.h \
  /cygdrive/d/sdk4.0ad.1.3_jade_c_20121207_16bit_new_new/inc/sys_parameters.h \
  /cygdrive/d/sdk4.0ad.1.3_jade_c_20121207_16bit_new_new/inc/api/libc/printf.h \
  /cygdrive/d/sdk4.0ad.1.3_jade_c_20121207_16bit_new_new/inc/osal/osal.h \
  /cygdrive/d/sdk4.0ad.1.3_jade_c_20121207_16bit_new_new/inc/retcode.h \
  /cygdrive/d/sdk4.0ad.1.3_jade_c_20121207_16bit_new_new/inc/types.h \
  /cygdrive/d/sdk4.0ad.1.3_jade_c_20121207_16bit_new_new/inc/basic_types.h \
  /cygdrive/d/sdk4.0ad.1.3_jade_c_20121207_16bit_new_new/inc/os/tds2/itron.h \
  /cygdrive/d/sdk4.0ad.1.3_jade_c_20121207_16bit_new_new/inc/os/tds2/alitypes.h \
  /cygdrive/d/sdk4.0ad.1.3_jade_c_20121207_16bit_new_new/inc/os/tds2/platform.h \
  /cygdrive/d/sdk4.0ad.1.3_jade_c_20121207_16bit_new_new/inc/os/tds2/config.h \
  /cygdrive/d/sdk4.0ad.1.3_jade_c_20121207_16bit_new_new/inc/osal/osal_task.h \
  /cygdrive/d/sdk4.0ad.1.3_jade_c_20121207_16bit_new_new/inc/osal/osal_int.h \
  /cygdrive/d/sdk4.0ad.1.3_jade_c_20121207_16bit_new_new/inc/osal/osal_timer.h \
  /cygdrive/d/sdk4.0ad.1.3_jade_c_20121207_16bit_new_new/inc/osal/osal_msgq.h \
  /cygdrive/d/sdk4.0ad.1.3_jade_c_20121207_16bit_new_new/inc/osal/osal_sema.h \
  /cygdrive/d/sdk4.0ad.1.3_jade_c_20121207_16bit_new_new/inc/osal/osal_flag.h \
  /cygdrive/d/sdk4.0ad.1.3_jade_c_20121207_16bit_new_new/inc/osal/osal_mm.h \
  /cygdrive/d/sdk4.0ad.1.3_jade_c_20121207_16bit_new_new/inc/api/libc/alloc.h \
  /cygdrive/d/sdk4.0ad.1.3_jade_c_20121207_16bit_new_new/inc/osal/osal_cache.h \
  /cygdrive/d/sdk4.0ad.1.3_jade_c_20121207_16bit_new_new/inc/osal/osal_mutex.h \
  /cygdrive/d/sdk4.0ad.1.3_jade_c_20121207_16bit_new_new/inc/hal/machine.h \
  /cygdrive/D/study/v2.1/ALi_IDE_2.1_compile_tools/compilers/gcc3.0.4/lib/gcc-lib/mips-t2-elf/3.0.4/include/stdarg.h \
  /cygdrive/d/sdk4.0ad.1.3_jade_c_20121207_16bit_new_new/inc/api/libc/string.h \
  /cygdrive/d/sdk4.0ad.1.3_jade_c_20121207_16bit_new_new/inc/api/libchunk/chunk.h \
  /cygdrive/d/sdk4.0ad.1.3_jade_c_20121207_16bit_new_new/inc/hld/sto/sto.h \
  /cygdrive/d/sdk4.0ad.1.3_jade_c_20121207_16bit_new_new/inc/hld/sto/sto_dev.h \
  /cygdrive/d/sdk4.0ad.1.3_jade_c_20121207_16bit_new_new/inc/hld/hld_dev.h \
  /cygdrive/d/sdk4.0ad.1.3_jade_c_20121207_16bit_new_new/inc/bus/erom/erom.h \
  /cygdrive/d/sdk4.0ad.1.3_jade_c_20121207_16bit_new_new/inc/bus/sci/sci.h \
  /cygdrive/d/sdk4.0ad.1.3_jade_c_20121207_16bit_new_new/inc/bus/flash/flash.h \
  /cygdrive/d/sdk4.0ad.1.3_jade_c_20121207_16bit_new_new/inc/hal/hal_gpio.h \
  /cygdrive/d/sdk4.0ad.1.3_jade_c_20121207_16bit_new_new/inc/hal/hal_mem.h \
  /cygdrive/d/sdk4.0ad.1.3_jade_c_20121207_16bit_new_new/inc/hld/pan/pan_dev.h \
  /cygdrive/d/sdk4.0ad.1.3_jade_c_20121207_16bit_new_new/inc/hld/pan/pan.h \
  /cygdrive/d/sdk4.0ad.1.3_jade_c_20121207_16bit_new_new/inc/api/libpub/lib_device_manage.h \
  /cygdrive/d/sdk4.0ad.1.3_jade_c_20121207_16bit_new_new/inc/api/libpub/lib_frontend.h \
  /cygdrive/d/sdk4.0ad.1.3_jade_c_20121207_16bit_new_new/inc/api/libtsi/p_search.h \
  /cygdrive/d/sdk4.0ad.1.3_jade_c_20121207_16bit_new_new/inc/api/libsi/si_config.h \
  /cygdrive/d/sdk4.0ad.1.3_jade_c_20121207_16bit_new_new/inc/api/libtsi/sec_pmt.h \
  /cygdrive/d/sdk4.0ad.1.3_jade_c_20121207_16bit_new_new/inc/api/libtsi/sec_sdt.h \
  /cygdrive/d/sdk4.0ad.1.3_jade_c_20121207_16bit_new_new/inc/api/libtsi/db_3l.h \
  /cygdrive/d/sdk4.0ad.1.3_jade_c_20121207_16bit_new_new/inc/api/libdb/db_return_value.h \
  /cygdrive/d/sdk4.0ad.1.3_jade_c_20121207_16bit_new_new/inc/api/libdb/db_config.h \
  /cygdrive/d/sdk4.0ad.1.3_jade_c_20121207_16bit_new_new/inc/api/libdb/db_node_c.h \
  /cygdrive/d/sdk4.0ad.1.3_jade_c_20121207_16bit_new_new/inc/api/libdb/db_interface.h \
  /cygdrive/d/sdk4.0ad.1.3_jade_c_20121207_16bit_new_new/inc/api/libloader/ld_boot_info.h \
  /cygdrive/d/sdk4.0ad.1.3_jade_c_20121207_16bit_new_new/inc/api/libstbinfo/stb_info_data.h \
  /cygdrive/d/sdk4.0ad.1.3_jade_c_20121207_16bit_new_new/inc/api/libupg/lib_upg4.h \
  /cygdrive/d/sdk4.0ad.1.3_jade_c_20121207_16bit_new_new/inc/api/libtsi/si_types.h \
  /cygdrive/d/sdk4.0ad.1.3_jade_c_20121207_16bit_new_new/inc/api/libupg/ota_upgrade.h \
  dev_handle.h upg_common.h ../copper_common/stb_data.h \
  /cygdrive/d/sdk4.0ad.1.3_jade_c_20121207_16bit_new_new/inc/mediatypes.h \
  /cygdrive/D/study/v2.1/ALi_IDE_2.1_compile_tools/compilers/gcc3.0.4/mips-t2-elf/include/math.h \
  /cygdrive/D/study/v2.1/ALi_IDE_2.1_compile_tools/compilers/gcc3.0.4/mips-t2-elf/include/sys/reent.h \
  /cygdrive/D/study/v2.1/ALi_IDE_2.1_compile_tools/compilers/gcc3.0.4/mips-t2-elf/include/_ansi.h \
  /cygdrive/D/study/v2.1/ALi_IDE_2.1_compile_tools/compilers/gcc3.0.4/mips-t2-elf/include/sys/config.h \
  /cygdrive/D/study/v2.1/ALi_IDE_2.1_compile_tools/compilers/gcc3.0.4/mips-t2-elf/include/time.h \
  /cygdrive/D/study/v2.1/ALi_IDE_2.1_compile_tools/compilers/gcc3.0.4/mips-t2-elf/include/machine/time.h \
  /cygdrive/D/study/v2.1/ALi_IDE_2.1_compile_tools/compilers/gcc3.0.4/lib/gcc-lib/mips-t2-elf/3.0.4/include/stddef.h \
  /cygdrive/D/study/v2.1/ALi_IDE_2.1_compile_tools/compilers/gcc3.0.4/mips-t2-elf/include/sys/types.h \
  /cygdrive/D/study/v2.1/ALi_IDE_2.1_compile_tools/compilers/gcc3.0.4/mips-t2-elf/include/machine/types.h \
  /cygdrive/D/study/v2.1/ALi_IDE_2.1_compile_tools/compilers/gcc3.0.4/mips-t2-elf/include/sys/features.h \
  /cygdrive/D/study/v2.1/ALi_IDE_2.1_compile_tools/compilers/gcc3.0.4/mips-t2-elf/include/machine/ieeefp.h \
  /cygdrive/d/sdk4.0ad.1.3_jade_c_20121207_16bit_new_new/inc/api/libstbinfo/stb_info.h \
  string.id
upg_menu.o: upg_menu.c \
  /cygdrive/d/sdk4.0ad.1.3_jade_c_20121207_16bit_new_new/prj/app/demo/dvbc/jade_demo_new_16bit/upg_loader/mcf/release/include/sys_config.h \
  /cygdrive/d/sdk4.0ad.1.3_jade_c_20121207_16bit_new_new/inc/sys_define.h \
  /cygdrive/d/sdk4.0ad.1.3_jade_c_20121207_16bit_new_new/prj/app/demo/dvbc/jade_demo_new_16bit/upg_loader/mcf/release/include/common_config.h \
  /cygdrive/d/sdk4.0ad.1.3_jade_c_20121207_16bit_new_new/inc/sys_parameters.h \
  /cygdrive/d/sdk4.0ad.1.3_jade_c_20121207_16bit_new_new/inc/api/libc/printf.h \
  /cygdrive/d/sdk4.0ad.1.3_jade_c_20121207_16bit_new_new/inc/osal/osal.h \
  /cygdrive/d/sdk4.0ad.1.3_jade_c_20121207_16bit_new_new/inc/retcode.h \
  /cygdrive/d/sdk4.0ad.1.3_jade_c_20121207_16bit_new_new/inc/types.h \
  /cygdrive/d/sdk4.0ad.1.3_jade_c_20121207_16bit_new_new/inc/basic_types.h \
  /cygdrive/d/sdk4.0ad.1.3_jade_c_20121207_16bit_new_new/inc/os/tds2/itron.h \
  /cygdrive/d/sdk4.0ad.1.3_jade_c_20121207_16bit_new_new/inc/os/tds2/alitypes.h \
  /cygdrive/d/sdk4.0ad.1.3_jade_c_20121207_16bit_new_new/inc/os/tds2/platform.h \
  /cygdrive/d/sdk4.0ad.1.3_jade_c_20121207_16bit_new_new/inc/os/tds2/config.h \
  /cygdrive/d/sdk4.0ad.1.3_jade_c_20121207_16bit_new_new/inc/osal/osal_task.h \
  /cygdrive/d/sdk4.0ad.1.3_jade_c_20121207_16bit_new_new/inc/osal/osal_int.h \
  /cygdrive/d/sdk4.0ad.1.3_jade_c_20121207_16bit_new_new/inc/osal/osal_timer.h \
  /cygdrive/d/sdk4.0ad.1.3_jade_c_20121207_16bit_new_new/inc/osal/osal_msgq.h \
  /cygdrive/d/sdk4.0ad.1.3_jade_c_20121207_16bit_new_new/inc/osal/osal_sema.h \
  /cygdrive/d/sdk4.0ad.1.3_jade_c_20121207_16bit_new_new/inc/osal/osal_flag.h \
  /cygdrive/d/sdk4.0ad.1.3_jade_c_20121207_16bit_new_new/inc/osal/osal_mm.h \
  /cygdrive/d/sdk4.0ad.1.3_jade_c_20121207_16bit_new_new/inc/api/libc/alloc.h \
  /cygdrive/d/sdk4.0ad.1.3_jade_c_20121207_16bit_new_new/inc/osal/osal_cache.h \
  /cygdrive/d/sdk4.0ad.1.3_jade_c_20121207_16bit_new_new/inc/osal/osal_mutex.h \
  /cygdrive/d/sdk4.0ad.1.3_jade_c_20121207_16bit_new_new/inc/hal/machine.h \
  /cygdrive/D/study/v2.1/ALi_IDE_2.1_compile_tools/compilers/gcc3.0.4/lib/gcc-lib/mips-t2-elf/3.0.4/include/stdarg.h \
  /cygdrive/d/sdk4.0ad.1.3_jade_c_20121207_16bit_new_new/inc/api/libc/string.h \
  /cygdrive/d/sdk4.0ad.1.3_jade_c_20121207_16bit_new_new/inc/api/libchunk/chunk.h \
  /cygdrive/d/sdk4.0ad.1.3_jade_c_20121207_16bit_new_new/inc/hld/sto/sto.h \
  /cygdrive/d/sdk4.0ad.1.3_jade_c_20121207_16bit_new_new/inc/hld/sto/sto_dev.h \
  /cygdrive/d/sdk4.0ad.1.3_jade_c_20121207_16bit_new_new/inc/hld/hld_dev.h \
  /cygdrive/d/sdk4.0ad.1.3_jade_c_20121207_16bit_new_new/inc/bus/erom/erom.h \
  /cygdrive/d/sdk4.0ad.1.3_jade_c_20121207_16bit_new_new/inc/bus/sci/sci.h \
  /cygdrive/d/sdk4.0ad.1.3_jade_c_20121207_16bit_new_new/inc/bus/flash/flash.h \
  /cygdrive/d/sdk4.0ad.1.3_jade_c_20121207_16bit_new_new/inc/hal/hal_gpio.h \
  /cygdrive/d/sdk4.0ad.1.3_jade_c_20121207_16bit_new_new/inc/hal/hal_mem.h \
  /cygdrive/d/sdk4.0ad.1.3_jade_c_20121207_16bit_new_new/inc/hld/pan/pan_dev.h \
  /cygdrive/d/sdk4.0ad.1.3_jade_c_20121207_16bit_new_new/inc/hld/pan/pan.h \
  /cygdrive/d/sdk4.0ad.1.3_jade_c_20121207_16bit_new_new/inc/api/libpub/lib_device_manage.h \
  /cygdrive/d/sdk4.0ad.1.3_jade_c_20121207_16bit_new_new/inc/api/libpub/lib_frontend.h \
  /cygdrive/d/sdk4.0ad.1.3_jade_c_20121207_16bit_new_new/inc/api/libtsi/p_search.h \
  /cygdrive/d/sdk4.0ad.1.3_jade_c_20121207_16bit_new_new/inc/api/libsi/si_config.h \
  /cygdrive/d/sdk4.0ad.1.3_jade_c_20121207_16bit_new_new/inc/api/libtsi/sec_pmt.h \
  /cygdrive/d/sdk4.0ad.1.3_jade_c_20121207_16bit_new_new/inc/api/libtsi/sec_sdt.h \
  /cygdrive/d/sdk4.0ad.1.3_jade_c_20121207_16bit_new_new/inc/api/libtsi/db_3l.h \
  /cygdrive/d/sdk4.0ad.1.3_jade_c_20121207_16bit_new_new/inc/api/libdb/db_return_value.h \
  /cygdrive/d/sdk4.0ad.1.3_jade_c_20121207_16bit_new_new/inc/api/libdb/db_config.h \
  /cygdrive/d/sdk4.0ad.1.3_jade_c_20121207_16bit_new_new/inc/api/libdb/db_node_c.h \
  /cygdrive/d/sdk4.0ad.1.3_jade_c_20121207_16bit_new_new/inc/api/libdb/db_interface.h \
  /cygdrive/d/sdk4.0ad.1.3_jade_c_20121207_16bit_new_new/inc/hld/osd/osddrv.h \
  /cygdrive/d/sdk4.0ad.1.3_jade_c_20121207_16bit_new_new/inc/hld/osd/osddrv_dev.h \
  /cygdrive/d/sdk4.0ad.1.3_jade_c_20121207_16bit_new_new/inc/api/libloader/ld_osd_lib.h \
  /cygdrive/d/sdk4.0ad.1.3_jade_c_20121207_16bit_new_new/inc/mediatypes.h \
  /cygdrive/d/sdk4.0ad.1.3_jade_c_20121207_16bit_new_new/inc/api/libchar/lib_char.h \
  /cygdrive/d/sdk4.0ad.1.3_jade_c_20121207_16bit_new_new/inc/api/libloader/ld_boot_info.h \
  /cygdrive/d/sdk4.0ad.1.3_jade_c_20121207_16bit_new_new/inc/api/libstbinfo/stb_info_data.h \
  /cygdrive/d/sdk4.0ad.1.3_jade_c_20121207_16bit_new_new/inc/api/libupg/lib_upg4.h \
  /cygdrive/d/sdk4.0ad.1.3_jade_c_20121207_16bit_new_new/inc/api/libtsi/si_types.h \
  /cygdrive/d/sdk4.0ad.1.3_jade_c_20121207_16bit_new_new/inc/api/libosd/osd_vkey.h \
  dev_handle.h upg_common.h ../copper_common/boot_system_data.h \
  ../copper_common/stb_data.h \
  /cygdrive/D/study/v2.1/ALi_IDE_2.1_compile_tools/compilers/gcc3.0.4/mips-t2-elf/include/math.h \
  /cygdrive/D/study/v2.1/ALi_IDE_2.1_compile_tools/compilers/gcc3.0.4/mips-t2-elf/include/sys/reent.h \
  /cygdrive/D/study/v2.1/ALi_IDE_2.1_compile_tools/compilers/gcc3.0.4/mips-t2-elf/include/_ansi.h \
  /cygdrive/D/study/v2.1/ALi_IDE_2.1_compile_tools/compilers/gcc3.0.4/mips-t2-elf/include/sys/config.h \
  /cygdrive/D/study/v2.1/ALi_IDE_2.1_compile_tools/compilers/gcc3.0.4/mips-t2-elf/include/time.h \
  /cygdrive/D/study/v2.1/ALi_IDE_2.1_compile_tools/compilers/gcc3.0.4/mips-t2-elf/include/machine/time.h \
  /cygdrive/D/study/v2.1/ALi_IDE_2.1_compile_tools/compilers/gcc3.0.4/lib/gcc-lib/mips-t2-elf/3.0.4/include/stddef.h \
  /cygdrive/D/study/v2.1/ALi_IDE_2.1_compile_tools/compilers/gcc3.0.4/mips-t2-elf/include/sys/types.h \
  /cygdrive/D/study/v2.1/ALi_IDE_2.1_compile_tools/compilers/gcc3.0.4/mips-t2-elf/include/machine/types.h \
  /cygdrive/D/study/v2.1/ALi_IDE_2.1_compile_tools/compilers/gcc3.0.4/mips-t2-elf/include/sys/features.h \
  /cygdrive/D/study/v2.1/ALi_IDE_2.1_compile_tools/compilers/gcc3.0.4/mips-t2-elf/include/machine/ieeefp.h \
  /cygdrive/d/sdk4.0ad.1.3_jade_c_20121207_16bit_new_new/inc/api/libstbinfo/stb_info.h \
  key.h string.id
board_config.o: board_config.c \
  /cygdrive/d/sdk4.0ad.1.3_jade_c_20121207_16bit_new_new/prj/app/demo/dvbc/jade_demo_new_16bit/upg_loader/mcf/release/include/sys_config.h \
  /cygdrive/d/sdk4.0ad.1.3_jade_c_20121207_16bit_new_new/inc/sys_define.h \
  /cygdrive/d/sdk4.0ad.1.3_jade_c_20121207_16bit_new_new/prj/app/demo/dvbc/jade_demo_new_16bit/upg_loader/mcf/release/include/common_config.h \
  /cygdrive/d/sdk4.0ad.1.3_jade_c_20121207_16bit_new_new/inc/sys_parameters.h \
  /cygdrive/d/sdk4.0ad.1.3_jade_c_20121207_16bit_new_new/inc/api/libc/printf.h \
  /cygdrive/d/sdk4.0ad.1.3_jade_c_20121207_16bit_new_new/inc/osal/osal.h \
  /cygdrive/d/sdk4.0ad.1.3_jade_c_20121207_16bit_new_new/inc/retcode.h \
  /cygdrive/d/sdk4.0ad.1.3_jade_c_20121207_16bit_new_new/inc/types.h \
  /cygdrive/d/sdk4.0ad.1.3_jade_c_20121207_16bit_new_new/inc/basic_types.h \
  /cygdrive/d/sdk4.0ad.1.3_jade_c_20121207_16bit_new_new/inc/os/tds2/itron.h \
  /cygdrive/d/sdk4.0ad.1.3_jade_c_20121207_16bit_new_new/inc/os/tds2/alitypes.h \
  /cygdrive/d/sdk4.0ad.1.3_jade_c_20121207_16bit_new_new/inc/os/tds2/platform.h \
  /cygdrive/d/sdk4.0ad.1.3_jade_c_20121207_16bit_new_new/inc/os/tds2/config.h \
  /cygdrive/d/sdk4.0ad.1.3_jade_c_20121207_16bit_new_new/inc/osal/osal_task.h \
  /cygdrive/d/sdk4.0ad.1.3_jade_c_20121207_16bit_new_new/inc/osal/osal_int.h \
  /cygdrive/d/sdk4.0ad.1.3_jade_c_20121207_16bit_new_new/inc/osal/osal_timer.h \
  /cygdrive/d/sdk4.0ad.1.3_jade_c_20121207_16bit_new_new/inc/osal/osal_msgq.h \
  /cygdrive/d/sdk4.0ad.1.3_jade_c_20121207_16bit_new_new/inc/osal/osal_sema.h \
  /cygdrive/d/sdk4.0ad.1.3_jade_c_20121207_16bit_new_new/inc/osal/osal_flag.h \
  /cygdrive/d/sdk4.0ad.1.3_jade_c_20121207_16bit_new_new/inc/osal/osal_mm.h \
  /cygdrive/d/sdk4.0ad.1.3_jade_c_20121207_16bit_new_new/inc/api/libc/alloc.h \
  /cygdrive/d/sdk4.0ad.1.3_jade_c_20121207_16bit_new_new/inc/osal/osal_cache.h \
  /cygdrive/d/sdk4.0ad.1.3_jade_c_20121207_16bit_new_new/inc/osal/osal_mutex.h \
  /cygdrive/d/sdk4.0ad.1.3_jade_c_20121207_16bit_new_new/inc/hal/machine.h \
  /cygdrive/D/study/v2.1/ALi_IDE_2.1_compile_tools/compilers/gcc3.0.4/lib/gcc-lib/mips-t2-elf/3.0.4/include/stdarg.h \
  /cygdrive/d/sdk4.0ad.1.3_jade_c_20121207_16bit_new_new/inc/api/libc/string.h \
  /cygdrive/d/sdk4.0ad.1.3_jade_c_20121207_16bit_new_new/inc/api/libchunk/chunk.h \
  /cygdrive/d/sdk4.0ad.1.3_jade_c_20121207_16bit_new_new/inc/hld/sto/sto.h \
  /cygdrive/d/sdk4.0ad.1.3_jade_c_20121207_16bit_new_new/inc/hld/sto/sto_dev.h \
  /cygdrive/d/sdk4.0ad.1.3_jade_c_20121207_16bit_new_new/inc/hld/hld_dev.h \
  /cygdrive/d/sdk4.0ad.1.3_jade_c_20121207_16bit_new_new/inc/bus/erom/erom.h \
  /cygdrive/d/sdk4.0ad.1.3_jade_c_20121207_16bit_new_new/inc/bus/sci/sci.h \
  /cygdrive/d/sdk4.0ad.1.3_jade_c_20121207_16bit_new_new/inc/bus/flash/flash.h \
  /cygdrive/d/sdk4.0ad.1.3_jade_c_20121207_16bit_new_new/inc/hal/hal_gpio.h \
  /cygdrive/d/sdk4.0ad.1.3_jade_c_20121207_16bit_new_new/inc/hal/hal_mem.h \
  /cygdrive/d/sdk4.0ad.1.3_jade_c_20121207_16bit_new_new/inc/hld/pan/pan_dev.h \
  /cygdrive/d/sdk4.0ad.1.3_jade_c_20121207_16bit_new_new/inc/hld/pan/pan.h \
  /cygdrive/d/sdk4.0ad.1.3_jade_c_20121207_16bit_new_new/inc/hld/nim/nim_tuner.h \
  /cygdrive/d/sdk4.0ad.1.3_jade_c_20121207_16bit_new_new/inc/hld/dmx/dmx.h \
  /cygdrive/d/sdk4.0ad.1.3_jade_c_20121207_16bit_new_new/inc/hld/dmx/dmx_dev.h \
  /cygdrive/d/sdk4.0ad.1.3_jade_c_20121207_16bit_new_new/inc/mediatypes.h \
  /cygdrive/d/sdk4.0ad.1.3_jade_c_20121207_16bit_new_new/inc/bus/dma/dma.h \
  /cygdrive/d/sdk4.0ad.1.3_jade_c_20121207_16bit_new_new/inc/bus/i2c/i2c.h \
  /cygdrive/d/sdk4.0ad.1.3_jade_c_20121207_16bit_new_new/inc/bus/tsi/tsi.h \
  /cygdrive/d/sdk4.0ad.1.3_jade_c_20121207_16bit_new_new/inc/hld/scart/scart.h \
  /cygdrive/d/sdk4.0ad.1.3_jade_c_20121207_16bit_new_new/inc/hld/osd/osddrv.h \
  /cygdrive/d/sdk4.0ad.1.3_jade_c_20121207_16bit_new_new/inc/hld/osd/osddrv_dev.h \
  /cygdrive/d/sdk4.0ad.1.3_jade_c_20121207_16bit_new_new/inc/api/libloader/ld_boot_info.h \
  /cygdrive/d/sdk4.0ad.1.3_jade_c_20121207_16bit_new_new/inc/api/libpub/lib_frontend.h \
  /cygdrive/d/sdk4.0ad.1.3_jade_c_20121207_16bit_new_new/inc/api/libtsi/p_search.h \
  /cygdrive/d/sdk4.0ad.1.3_jade_c_20121207_16bit_new_new/inc/api/libsi/si_config.h \
  /cygdrive/d/sdk4.0ad.1.3_jade_c_20121207_16bit_new_new/inc/api/libtsi/sec_pmt.h \
  /cygdrive/d/sdk4.0ad.1.3_jade_c_20121207_16bit_new_new/inc/api/libtsi/sec_sdt.h \
  /cygdrive/d/sdk4.0ad.1.3_jade_c_20121207_16bit_new_new/inc/api/libtsi/db_3l.h \
  /cygdrive/d/sdk4.0ad.1.3_jade_c_20121207_16bit_new_new/inc/api/libdb/db_return_value.h \
  /cygdrive/d/sdk4.0ad.1.3_jade_c_20121207_16bit_new_new/inc/api/libdb/db_config.h \
  /cygdrive/d/sdk4.0ad.1.3_jade_c_20121207_16bit_new_new/inc/api/libdb/db_node_c.h \
  /cygdrive/d/sdk4.0ad.1.3_jade_c_20121207_16bit_new_new/inc/api/libdb/db_interface.h \
  /cygdrive/d/sdk4.0ad.1.3_jade_c_20121207_16bit_new_new/inc/hld/dis/vpo.h \
  /cygdrive/d/sdk4.0ad.1.3_jade_c_20121207_16bit_new_new/inc/bus/hdmi/m36/hdmi_api.h \
  /cygdrive/d/sdk4.0ad.1.3_jade_c_20121207_16bit_new_new/inc/hld/ge/ge.h \
  /cygdrive/d/sdk4.0ad.1.3_jade_c_20121207_16bit_new_new/inc/hld/ge/ge_old.h \
  /cygdrive/d/sdk4.0ad.1.3_jade_c_20121207_16bit_new_new/inc/hld/ge/ge_new.h \
  /cygdrive/d/sdk4.0ad.1.3_jade_c_20121207_16bit_new_new/inc/hld/ge/gma_new.h \
  /cygdrive/d/sdk4.0ad.1.3_jade_c_20121207_16bit_new_new/inc/hld/decv/decv.h \
  /cygdrive/d/sdk4.0ad.1.3_jade_c_20121207_16bit_new_new/inc/hld/decv/decv_avc.h \
  /cygdrive/d/sdk4.0ad.1.3_jade_c_20121207_16bit_new_new/inc/hld/nim/nim_dev.h \
  /cygdrive/d/sdk4.0ad.1.3_jade_c_20121207_16bit_new_new/inc/api/libstbinfo/stb_info_data.h \
  dev_handle.h upg_common.h ../copper_common/boot_system_data.h \
  ../copper_common/stb_data.h \
  /cygdrive/D/study/v2.1/ALi_IDE_2.1_compile_tools/compilers/gcc3.0.4/mips-t2-elf/include/math.h \
  /cygdrive/D/study/v2.1/ALi_IDE_2.1_compile_tools/compilers/gcc3.0.4/mips-t2-elf/include/sys/reent.h \
  /cygdrive/D/study/v2.1/ALi_IDE_2.1_compile_tools/compilers/gcc3.0.4/mips-t2-elf/include/_ansi.h \
  /cygdrive/D/study/v2.1/ALi_IDE_2.1_compile_tools/compilers/gcc3.0.4/mips-t2-elf/include/sys/config.h \
  /cygdrive/D/study/v2.1/ALi_IDE_2.1_compile_tools/compilers/gcc3.0.4/mips-t2-elf/include/time.h \
  /cygdrive/D/study/v2.1/ALi_IDE_2.1_compile_tools/compilers/gcc3.0.4/mips-t2-elf/include/machine/time.h \
  /cygdrive/D/study/v2.1/ALi_IDE_2.1_compile_tools/compilers/gcc3.0.4/lib/gcc-lib/mips-t2-elf/3.0.4/include/stddef.h \
  /cygdrive/D/study/v2.1/ALi_IDE_2.1_compile_tools/compilers/gcc3.0.4/mips-t2-elf/include/sys/types.h \
  /cygdrive/D/study/v2.1/ALi_IDE_2.1_compile_tools/compilers/gcc3.0.4/mips-t2-elf/include/machine/types.h \
  /cygdrive/D/study/v2.1/ALi_IDE_2.1_compile_tools/compilers/gcc3.0.4/mips-t2-elf/include/sys/features.h \
  /cygdrive/D/study/v2.1/ALi_IDE_2.1_compile_tools/compilers/gcc3.0.4/mips-t2-elf/include/machine/ieeefp.h \
  /cygdrive/d/sdk4.0ad.1.3_jade_c_20121207_16bit_new_new/inc/api/libstbinfo/stb_info.h \
  key.h
