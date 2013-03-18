
##############################################
#
#    Makefile for your library
#        Ali sdk auto generated. Don't edit it!
#                            ALI corporation.
#
##############################################



# Environment settings
GCCROOT = /D/ALi_IDE_2.1/compilers/gcc3.0.4/
export PATH:=/bin:/cygdrive/C/Ali_SDK/bin/:$(PATH)

# gcc prefix
Prefix = mips-t2-elf-

CC = $(GCCROOT)bin/$(Prefix)gcc.exe
CXX = $(GCCROOT)bin/$(Prefix)g++.exe
LD = $(GCCROOT)bin/$(Prefix)ld.exe
OBJCOPY = $(GCCROOT)bin/$(Prefix)objcopy.exe
OBJDUMP = $(GCCROOT)bin/$(Prefix)objdump.exe
AR = $(GCCROOT)bin/$(Prefix)ar.exe

CP = cp
ECHO = echo
MAKE = make

# Parameters
ccflag =  -O1 -mips2 -EL -D_DEBUG_VERSION_ -DALi_M6303_Module_1 -W -I$(release_path)/include/ -I$(REPOSITORY)/inc -g -msoft-float -fno-delayed-branch

asflag =  -O1 -mips2 -EL -D_DEBUG_VERSION_ -DALi_M6303_Module_1 -W -I$(release_path)/include/ -I$(REPOSITORY)/inc -g -msoft-float -fno-delayed-branch

# Repository path which contains the root module,
# is root for all modules' source files.
REPOSITORY = /d/3202c-2013.3.4/

# APP_ROOT_PATH is application root path,
# all app files are under this root.
APP_ROOT_PATH = /d/3202c-2013.3.4/prj/app/demo/dvbc/jade_demo_new_geeya/

# project release path.
RELEASE_PROJECT_PATH = xxx

# build path which architecture same as source, 
# store .o and .d files along with tmp subdirectory.
build_path = build
# release path contains include and lib sub-directories 
release_path = release

# Custom Predefine:  


# List of source files
SRCS := 
SRCCS := 
ASMS := 
OFILES := 
LIBS := 
SRCS_D := 
SRCCS_D := 
ASMS_D := 
MDFS :=

# Modules selection 
M_m3327 := y
M_BUS := y
M_BUS_DOG :=
M_BUS_DOG_M33 :=
M_BUS_I2C := y
M_BUS_I2C_GPIO := y
M_BUS_I2C_SCB := y
M_BUS_SCI := y
M_BUS_SCI_UART := y
M_HLD := y
M_HLD_DECA := y
M_HLD_DECV := y
M_HLD_DMX := y
M_HLD_GE := y
M_HLD_NIM := y
M_HLD_OSD := y
M_HLD_PAN := y
M_HLD_SMC := y
M_HLD_SND := y
M_HLD_STO := y
M_HLD_VENC := y
M_HLD_VP := y
M_LIB := y
M_LIB_APPLET := y
M_LIB_BOOTUPG3 := y
M_LIB_MP := y
M_LIB_IMAGEDEC := y
M_LIB_GIFDEC := y
M_LIB_JPEGDEC := y
M_LIB_PNGDEC := y
M_LIB_PUB := y
M_LIB_PUB_ALL := y
M_LIB_PUB_AS_C := y
M_LIB_PUB_AS_ENGINE := y
M_LIB_UPG4 :=
M_LIB_UPG4_OTA :=
M_LIB_CAS := y
M_CDCA :=
M_DVTCA :=
M_GYCA := y
M_LIB_COMMON := y
M_LIB_C := y
M_LIB_CHAR := y
M_LIB_ZIP := y
M_LIB_7ZIP := y
M_LIB_GNUZIP :=
M_LIB_DVB := y
M_LIB_DBC := y
M_LIB_EPG := y
M_LIB_NVOD := y
M_LIB_SI := y
M_LIB_TSI3 := y
M_LIB_TSI3_CON_PSI := y
M_LIB_TSI3_DESCRIPTOR := y
M_LIB_TSI3_ENGINE := y
M_LIB_TSI3_PARSER := y
M_LIB_TSI3_PLUGIN_EIT := y
M_LIB_TSI3_PLUGIN_MONITOR := y
M_LIB_TSI3_PLUGIN_SEARCH := y
M_LIB_TSI3_PLUGIN_TDT := y
M_LIB_TSI3_UTILITY := y
M_LIBOTA_DVBC := y
M_LIB_LOADER :=
M_LIB_STORAGE := y
M_LIB_CHUNK := y
M_LIB_DATABASE := y
M_LIB_DB4 := y
M_LIB_DB4_ENGINE := y
M_LIB_DB4_INTERFACE := y
M_LIB_DB4_PLUGIN_C := y
M_LIB_STBINFO := y
M_LIB_UI := y
M_LIB_GE := y
M_LLD := y
M_LLD_NIM := y
M_LLD_NIM_C := y
M_LLD_TUN_C_RT820 := y
M_LLD_PAN := y
M_LLD_IRC := y
M_LLD_IRC_M63 := y
M_LLD_PAN_CT1642 := y
M_LLD_PAN_TP6X0 := y
M_LLD_SMC := y
M_LLD_SMARTCARD := y
M_LLD_STO := y
M_LLD_STO_FLASH := y
M_LLD_VENC := y
M_LLD_VENC_M33 := y
M_UDI :=
M_UDI_IPANEL :=
M_VERSION_INFO :=


# Modules detail 
.PHONY : M_m3327
ifeq ($(M_m3327),y)
varcs :=
varccs :=
varasms :=
mdfs := \
	 ${REPOSITORY}/m3327.mdf
varofiles := 
varlibs := 
SRCS += $(varcs)
SRCCS += $(varccs)
ASMS += $(varasms)
MDFS +=  $(mdfs)
OFILES += $(varofiles)
LIBS += $(varlibs)
M_m3327: M_BUS M_HLD M_LIB M_LLD M_UDI M_VERSION_INFO
else
M_m3327 :
endif

.PHONY : M_BUS
ifeq ($(M_BUS),y)
varcs :=
varccs :=
varasms :=
mdfs := \
	 ${REPOSITORY}/SRC/bus/bus.mdf
varofiles := 
varlibs := 
SRCS += $(varcs)
SRCCS += $(varccs)
ASMS += $(varasms)
MDFS +=  $(mdfs)
OFILES += $(varofiles)
LIBS += $(varlibs)
M_BUS: M_BUS_DOG M_BUS_I2C M_BUS_SCI
else
M_BUS :
endif

.PHONY : M_BUS_DOG
ifeq ($(M_BUS_DOG),y)
varcs :=
varccs :=
varasms :=
mdfs := \
	 ${REPOSITORY}/SRC/bus/dog/bus_dog.mdf
varofiles := 
varlibs := 
SRCS += $(varcs)
SRCCS += $(varccs)
ASMS += $(varasms)
MDFS +=  $(mdfs)
OFILES += $(varofiles)
LIBS += $(varlibs)
M_BUS_DOG: M_BUS_DOG_M33
else
M_BUS_DOG :
endif

.PHONY : M_BUS_DOG_M33
ifeq ($(M_BUS_DOG_M33),y)
varcs := \
	 $(REPOSITORY)/SRC/bus/dog/m33//dog_m33.c
varccs :=
varasms :=
mdfs := \
	 ${REPOSITORY}/SRC/bus/dog/m33/bus_dog_m33.mdf
varofiles := 
varlibs := 
SRCS += $(varcs)
SRCCS += $(varccs)
ASMS += $(varasms)
MDFS +=  $(mdfs)
OFILES += $(varofiles)
LIBS += $(varlibs)
M_BUS_DOG_M33: ${varasms:$(REPOSITORY)/%.S=$(build_path)/%.o} ${varcs:$(REPOSITORY)/%.c=$(build_path)/%.o} ${varccs:$(REPOSITORY)/%.cpp=$(build_path)/%.o}
else
M_BUS_DOG_M33 :
endif

.PHONY : M_BUS_I2C
ifeq ($(M_BUS_I2C),y)
varcs :=
varccs :=
varasms :=
mdfs := \
	 ${REPOSITORY}/SRC/bus/i2c/bus_i2c.mdf
varofiles := 
varlibs := 
SRCS += $(varcs)
SRCCS += $(varccs)
ASMS += $(varasms)
MDFS +=  $(mdfs)
OFILES += $(varofiles)
LIBS += $(varlibs)
M_BUS_I2C: M_BUS_I2C_GPIO M_BUS_I2C_SCB
else
M_BUS_I2C :
endif

.PHONY : M_BUS_I2C_GPIO
ifeq ($(M_BUS_I2C_GPIO),y)
varcs := \
	 $(REPOSITORY)/SRC/bus/i2c/gpio//i2c_gpio.c
varccs :=
varasms :=
mdfs := \
	 ${REPOSITORY}/SRC/bus/i2c/gpio/bus_i2c_gpio.mdf
varofiles := 
varlibs := 
SRCS += $(varcs)
SRCCS += $(varccs)
ASMS += $(varasms)
MDFS +=  $(mdfs)
OFILES += $(varofiles)
LIBS += $(varlibs)
M_BUS_I2C_GPIO: ${varasms:$(REPOSITORY)/%.S=$(build_path)/%.o} ${varcs:$(REPOSITORY)/%.c=$(build_path)/%.o} ${varccs:$(REPOSITORY)/%.cpp=$(build_path)/%.o}
else
M_BUS_I2C_GPIO :
endif

.PHONY : M_BUS_I2C_SCB
ifeq ($(M_BUS_I2C_SCB),y)
varcs := \
	 $(REPOSITORY)/SRC/bus/i2c/scb//i2c_scb.c
varccs :=
varasms :=
mdfs := \
	 ${REPOSITORY}/SRC/bus/i2c/scb/bus_i2c_scb.mdf
varofiles := 
varlibs := 
SRCS += $(varcs)
SRCCS += $(varccs)
ASMS += $(varasms)
MDFS +=  $(mdfs)
OFILES += $(varofiles)
LIBS += $(varlibs)
M_BUS_I2C_SCB: ${varasms:$(REPOSITORY)/%.S=$(build_path)/%.o} ${varcs:$(REPOSITORY)/%.c=$(build_path)/%.o} ${varccs:$(REPOSITORY)/%.cpp=$(build_path)/%.o}
else
M_BUS_I2C_SCB :
endif

.PHONY : M_BUS_SCI
ifeq ($(M_BUS_SCI),y)
varcs :=
varccs :=
varasms :=
mdfs := \
	 ${REPOSITORY}/SRC/bus/sci/bus_sci.mdf
varofiles := 
varlibs := 
SRCS += $(varcs)
SRCCS += $(varccs)
ASMS += $(varasms)
MDFS +=  $(mdfs)
OFILES += $(varofiles)
LIBS += $(varlibs)
M_BUS_SCI: M_BUS_SCI_UART
else
M_BUS_SCI :
endif

.PHONY : M_BUS_SCI_UART
ifeq ($(M_BUS_SCI_UART),y)
varcs := \
	 $(REPOSITORY)/SRC/bus/sci/uart//sci_16550uart.c
varccs :=
varasms :=
mdfs := \
	 ${REPOSITORY}/SRC/bus/sci/uart/bus_sci_uart.mdf
varofiles := 
varlibs := 
SRCS += $(varcs)
SRCCS += $(varccs)
ASMS += $(varasms)
MDFS +=  $(mdfs)
OFILES += $(varofiles)
LIBS += $(varlibs)
M_BUS_SCI_UART: ${varasms:$(REPOSITORY)/%.S=$(build_path)/%.o} ${varcs:$(REPOSITORY)/%.c=$(build_path)/%.o} ${varccs:$(REPOSITORY)/%.cpp=$(build_path)/%.o}
else
M_BUS_SCI_UART :
endif

.PHONY : M_HLD
ifeq ($(M_HLD),y)
varcs := \
	 $(REPOSITORY)/SRC/hld//hld_dev.c
varccs :=
varasms :=
mdfs := \
	 ${REPOSITORY}/SRC/hld/hld.mdf
varofiles := 
varlibs := 
SRCS += $(varcs)
SRCCS += $(varccs)
ASMS += $(varasms)
MDFS +=  $(mdfs)
OFILES += $(varofiles)
LIBS += $(varlibs)
M_HLD: M_HLD_DECA M_HLD_DECV M_HLD_DMX M_HLD_GE M_HLD_NIM M_HLD_OSD M_HLD_PAN M_HLD_SMC M_HLD_SND M_HLD_STO M_HLD_VENC M_HLD_VP ${varasms:$(REPOSITORY)/%.S=$(build_path)/%.o} ${varcs:$(REPOSITORY)/%.c=$(build_path)/%.o} ${varccs:$(REPOSITORY)/%.cpp=$(build_path)/%.o}
# Special compile
${varcs:$(REPOSITORY)/%.c=$(build_path)/%.o} :
	@$(CC) $(ccflag)  -I$(REPOSITORY)/inc/hal/ -I$(REPOSITORY)/inc/os/tds2/ -c $< -g -o $@
${varccs:$(REPOSITORY)/%.cpp=$(build_path)/%.o} :
	@$(CXX) $(ccflag)  -I$(REPOSITORY)/inc/hal/ -I$(REPOSITORY)/inc/os/tds2/ -c $< -g -o $@
${varasms:$(REPOSITORY)/%.S=$(build_path)/%.o} : $(build_path)/%.o : $(REPOSITORY)/%.S
	@$(CC) $(ccflag)  -I$(REPOSITORY)/inc/hal/ -I$(REPOSITORY)/inc/os/tds2/ -c $< -g -o $@
${varcs:$(REPOSITORY)/%.c=$(build_path)/%.d} : $(build_path)/%.d : $(REPOSITORY)/%.c
	@$(call DEPENDENCY_DEDUCE,$@,$<,$(ccflag)  -I$(REPOSITORY)/inc/hal/ -I$(REPOSITORY)/inc/os/tds2/)
${varccs:$(REPOSITORY)/%.cpp=$(build_path)/%.d} : $(build_path)/%.d : $(REPOSITORY)/%.cpp
	@$(call DEPENDENCY_DEDUCE,$@,$<,$(ccflag)  -I$(REPOSITORY)/inc/hal/ -I$(REPOSITORY)/inc/os/tds2/)
${varasms:$(REPOSITORY)/%.S=$(build_path)/%.d} : $(build_path)/%.d : $(REPOSITORY)/%.S
	@$(call DEPENDENCY_DEDUCE,$@,$<,$(ccflag)-I$(REPOSITORY)/inc/hal/ -I$(REPOSITORY)/inc/os/tds2/)
else
M_HLD :
endif

.PHONY : M_HLD_DECA
ifeq ($(M_HLD_DECA),y)
varcs := \
	 $(REPOSITORY)/SRC/hld/deca//deca.c
varccs :=
varasms :=
mdfs := \
	 ${REPOSITORY}/SRC/hld/deca/hld_deca.mdf
varofiles := 
varlibs := 
SRCS += $(varcs)
SRCCS += $(varccs)
ASMS += $(varasms)
MDFS +=  $(mdfs)
OFILES += $(varofiles)
LIBS += $(varlibs)
M_HLD_DECA: ${varasms:$(REPOSITORY)/%.S=$(build_path)/%.o} ${varcs:$(REPOSITORY)/%.c=$(build_path)/%.o} ${varccs:$(REPOSITORY)/%.cpp=$(build_path)/%.o}
# Special compile
${varcs:$(REPOSITORY)/%.c=$(build_path)/%.o} :
	@$(CC) $(ccflag)  -I$(REPOSITORY)/inc/hal/ -I$(REPOSITORY)/inc/os/tds2/ -c $< -g -o $@
${varccs:$(REPOSITORY)/%.cpp=$(build_path)/%.o} :
	@$(CXX) $(ccflag)  -I$(REPOSITORY)/inc/hal/ -I$(REPOSITORY)/inc/os/tds2/ -c $< -g -o $@
${varasms:$(REPOSITORY)/%.S=$(build_path)/%.o} : $(build_path)/%.o : $(REPOSITORY)/%.S
	@$(CC) $(ccflag)  -I$(REPOSITORY)/inc/hal/ -I$(REPOSITORY)/inc/os/tds2/ -c $< -g -o $@
${varcs:$(REPOSITORY)/%.c=$(build_path)/%.d} : $(build_path)/%.d : $(REPOSITORY)/%.c
	@$(call DEPENDENCY_DEDUCE,$@,$<,$(ccflag)  -I$(REPOSITORY)/inc/hal/ -I$(REPOSITORY)/inc/os/tds2/)
${varccs:$(REPOSITORY)/%.cpp=$(build_path)/%.d} : $(build_path)/%.d : $(REPOSITORY)/%.cpp
	@$(call DEPENDENCY_DEDUCE,$@,$<,$(ccflag)  -I$(REPOSITORY)/inc/hal/ -I$(REPOSITORY)/inc/os/tds2/)
${varasms:$(REPOSITORY)/%.S=$(build_path)/%.d} : $(build_path)/%.d : $(REPOSITORY)/%.S
	@$(call DEPENDENCY_DEDUCE,$@,$<,$(ccflag)-I$(REPOSITORY)/inc/hal/ -I$(REPOSITORY)/inc/os/tds2/)
else
M_HLD_DECA :
endif

.PHONY : M_HLD_DECV
ifeq ($(M_HLD_DECV),y)
varcs := \
	 $(REPOSITORY)/SRC/hld/decv//decv.c
varccs :=
varasms :=
mdfs := \
	 ${REPOSITORY}/SRC/hld/decv/hld_decv.mdf
varofiles := 
varlibs := 
SRCS += $(varcs)
SRCCS += $(varccs)
ASMS += $(varasms)
MDFS +=  $(mdfs)
OFILES += $(varofiles)
LIBS += $(varlibs)
M_HLD_DECV: ${varasms:$(REPOSITORY)/%.S=$(build_path)/%.o} ${varcs:$(REPOSITORY)/%.c=$(build_path)/%.o} ${varccs:$(REPOSITORY)/%.cpp=$(build_path)/%.o}
# Special compile
${varcs:$(REPOSITORY)/%.c=$(build_path)/%.o} :
	@$(CC) $(ccflag)  -I$(REPOSITORY)/inc/hal/ -I$(REPOSITORY)/inc/os/tds2/ -c $< -g -o $@
${varccs:$(REPOSITORY)/%.cpp=$(build_path)/%.o} :
	@$(CXX) $(ccflag)  -I$(REPOSITORY)/inc/hal/ -I$(REPOSITORY)/inc/os/tds2/ -c $< -g -o $@
${varasms:$(REPOSITORY)/%.S=$(build_path)/%.o} : $(build_path)/%.o : $(REPOSITORY)/%.S
	@$(CC) $(ccflag)  -I$(REPOSITORY)/inc/hal/ -I$(REPOSITORY)/inc/os/tds2/ -c $< -g -o $@
${varcs:$(REPOSITORY)/%.c=$(build_path)/%.d} : $(build_path)/%.d : $(REPOSITORY)/%.c
	@$(call DEPENDENCY_DEDUCE,$@,$<,$(ccflag)  -I$(REPOSITORY)/inc/hal/ -I$(REPOSITORY)/inc/os/tds2/)
${varccs:$(REPOSITORY)/%.cpp=$(build_path)/%.d} : $(build_path)/%.d : $(REPOSITORY)/%.cpp
	@$(call DEPENDENCY_DEDUCE,$@,$<,$(ccflag)  -I$(REPOSITORY)/inc/hal/ -I$(REPOSITORY)/inc/os/tds2/)
${varasms:$(REPOSITORY)/%.S=$(build_path)/%.d} : $(build_path)/%.d : $(REPOSITORY)/%.S
	@$(call DEPENDENCY_DEDUCE,$@,$<,$(ccflag)-I$(REPOSITORY)/inc/hal/ -I$(REPOSITORY)/inc/os/tds2/)
else
M_HLD_DECV :
endif

.PHONY : M_HLD_DMX
ifeq ($(M_HLD_DMX),y)
varcs := \
	 $(REPOSITORY)/SRC/hld/dmx//dmx.c \
	 $(REPOSITORY)/SRC/hld/dmx//dmx_adv.c \
	 $(REPOSITORY)/SRC/hld/dmx//dmx_adv_slot.c
varccs :=
varasms :=
mdfs := \
	 ${REPOSITORY}/SRC/hld/dmx/hld_dmx.mdf
varofiles := 
varlibs := 
SRCS += $(varcs)
SRCCS += $(varccs)
ASMS += $(varasms)
MDFS +=  $(mdfs)
OFILES += $(varofiles)
LIBS += $(varlibs)
M_HLD_DMX: ${varasms:$(REPOSITORY)/%.S=$(build_path)/%.o} ${varcs:$(REPOSITORY)/%.c=$(build_path)/%.o} ${varccs:$(REPOSITORY)/%.cpp=$(build_path)/%.o}
# Special compile
${varcs:$(REPOSITORY)/%.c=$(build_path)/%.o} :
	@$(CC) $(ccflag)  -I$(REPOSITORY)/inc/hal/ -I$(REPOSITORY)/inc/os/tds2/ -c $< -g -o $@
${varccs:$(REPOSITORY)/%.cpp=$(build_path)/%.o} :
	@$(CXX) $(ccflag)  -I$(REPOSITORY)/inc/hal/ -I$(REPOSITORY)/inc/os/tds2/ -c $< -g -o $@
${varasms:$(REPOSITORY)/%.S=$(build_path)/%.o} : $(build_path)/%.o : $(REPOSITORY)/%.S
	@$(CC) $(ccflag)  -I$(REPOSITORY)/inc/hal/ -I$(REPOSITORY)/inc/os/tds2/ -c $< -g -o $@
${varcs:$(REPOSITORY)/%.c=$(build_path)/%.d} : $(build_path)/%.d : $(REPOSITORY)/%.c
	@$(call DEPENDENCY_DEDUCE,$@,$<,$(ccflag)  -I$(REPOSITORY)/inc/hal/ -I$(REPOSITORY)/inc/os/tds2/)
${varccs:$(REPOSITORY)/%.cpp=$(build_path)/%.d} : $(build_path)/%.d : $(REPOSITORY)/%.cpp
	@$(call DEPENDENCY_DEDUCE,$@,$<,$(ccflag)  -I$(REPOSITORY)/inc/hal/ -I$(REPOSITORY)/inc/os/tds2/)
${varasms:$(REPOSITORY)/%.S=$(build_path)/%.d} : $(build_path)/%.d : $(REPOSITORY)/%.S
	@$(call DEPENDENCY_DEDUCE,$@,$<,$(ccflag)-I$(REPOSITORY)/inc/hal/ -I$(REPOSITORY)/inc/os/tds2/)
else
M_HLD_DMX :
endif

.PHONY : M_HLD_GE
ifeq ($(M_HLD_GE),y)
varcs := \
	 $(REPOSITORY)/SRC/hld/ge//ge.c
varccs :=
varasms :=
mdfs := \
	 ${REPOSITORY}/SRC/hld/ge/hld_ge.mdf
varofiles := 
varlibs := 
SRCS += $(varcs)
SRCCS += $(varccs)
ASMS += $(varasms)
MDFS +=  $(mdfs)
OFILES += $(varofiles)
LIBS += $(varlibs)
M_HLD_GE: ${varasms:$(REPOSITORY)/%.S=$(build_path)/%.o} ${varcs:$(REPOSITORY)/%.c=$(build_path)/%.o} ${varccs:$(REPOSITORY)/%.cpp=$(build_path)/%.o}
# Special compile
${varcs:$(REPOSITORY)/%.c=$(build_path)/%.o} :
	@$(CC) $(ccflag)  -I$(REPOSITORY)/inc/hal/ -I$(REPOSITORY)/inc/os/tds2/ -c $< -g -o $@
${varccs:$(REPOSITORY)/%.cpp=$(build_path)/%.o} :
	@$(CXX) $(ccflag)  -I$(REPOSITORY)/inc/hal/ -I$(REPOSITORY)/inc/os/tds2/ -c $< -g -o $@
${varasms:$(REPOSITORY)/%.S=$(build_path)/%.o} : $(build_path)/%.o : $(REPOSITORY)/%.S
	@$(CC) $(ccflag)  -I$(REPOSITORY)/inc/hal/ -I$(REPOSITORY)/inc/os/tds2/ -c $< -g -o $@
${varcs:$(REPOSITORY)/%.c=$(build_path)/%.d} : $(build_path)/%.d : $(REPOSITORY)/%.c
	@$(call DEPENDENCY_DEDUCE,$@,$<,$(ccflag)  -I$(REPOSITORY)/inc/hal/ -I$(REPOSITORY)/inc/os/tds2/)
${varccs:$(REPOSITORY)/%.cpp=$(build_path)/%.d} : $(build_path)/%.d : $(REPOSITORY)/%.cpp
	@$(call DEPENDENCY_DEDUCE,$@,$<,$(ccflag)  -I$(REPOSITORY)/inc/hal/ -I$(REPOSITORY)/inc/os/tds2/)
${varasms:$(REPOSITORY)/%.S=$(build_path)/%.d} : $(build_path)/%.d : $(REPOSITORY)/%.S
	@$(call DEPENDENCY_DEDUCE,$@,$<,$(ccflag)-I$(REPOSITORY)/inc/hal/ -I$(REPOSITORY)/inc/os/tds2/)
else
M_HLD_GE :
endif

.PHONY : M_HLD_NIM
ifeq ($(M_HLD_NIM),y)
varcs := \
	 $(REPOSITORY)/SRC/hld/nim//nim.c
varccs :=
varasms :=
mdfs := \
	 ${REPOSITORY}/SRC/hld/nim/hld_nim.mdf
varofiles := 
varlibs := 
SRCS += $(varcs)
SRCCS += $(varccs)
ASMS += $(varasms)
MDFS +=  $(mdfs)
OFILES += $(varofiles)
LIBS += $(varlibs)
M_HLD_NIM: ${varasms:$(REPOSITORY)/%.S=$(build_path)/%.o} ${varcs:$(REPOSITORY)/%.c=$(build_path)/%.o} ${varccs:$(REPOSITORY)/%.cpp=$(build_path)/%.o}
# Special compile
${varcs:$(REPOSITORY)/%.c=$(build_path)/%.o} :
	@$(CC) $(ccflag)  -I$(REPOSITORY)/inc/hal/ -I$(REPOSITORY)/inc/os/tds2/ -c $< -g -o $@
${varccs:$(REPOSITORY)/%.cpp=$(build_path)/%.o} :
	@$(CXX) $(ccflag)  -I$(REPOSITORY)/inc/hal/ -I$(REPOSITORY)/inc/os/tds2/ -c $< -g -o $@
${varasms:$(REPOSITORY)/%.S=$(build_path)/%.o} : $(build_path)/%.o : $(REPOSITORY)/%.S
	@$(CC) $(ccflag)  -I$(REPOSITORY)/inc/hal/ -I$(REPOSITORY)/inc/os/tds2/ -c $< -g -o $@
${varcs:$(REPOSITORY)/%.c=$(build_path)/%.d} : $(build_path)/%.d : $(REPOSITORY)/%.c
	@$(call DEPENDENCY_DEDUCE,$@,$<,$(ccflag)  -I$(REPOSITORY)/inc/hal/ -I$(REPOSITORY)/inc/os/tds2/)
${varccs:$(REPOSITORY)/%.cpp=$(build_path)/%.d} : $(build_path)/%.d : $(REPOSITORY)/%.cpp
	@$(call DEPENDENCY_DEDUCE,$@,$<,$(ccflag)  -I$(REPOSITORY)/inc/hal/ -I$(REPOSITORY)/inc/os/tds2/)
${varasms:$(REPOSITORY)/%.S=$(build_path)/%.d} : $(build_path)/%.d : $(REPOSITORY)/%.S
	@$(call DEPENDENCY_DEDUCE,$@,$<,$(ccflag)-I$(REPOSITORY)/inc/hal/ -I$(REPOSITORY)/inc/os/tds2/)
else
M_HLD_NIM :
endif

.PHONY : M_HLD_OSD
ifeq ($(M_HLD_OSD),y)
varcs := \
	 $(REPOSITORY)/SRC/hld/osd//osddrv.c
varccs :=
varasms :=
mdfs := \
	 ${REPOSITORY}/SRC/hld/osd/hld_osd.mdf
varofiles := 
varlibs := 
SRCS += $(varcs)
SRCCS += $(varccs)
ASMS += $(varasms)
MDFS +=  $(mdfs)
OFILES += $(varofiles)
LIBS += $(varlibs)
M_HLD_OSD: ${varasms:$(REPOSITORY)/%.S=$(build_path)/%.o} ${varcs:$(REPOSITORY)/%.c=$(build_path)/%.o} ${varccs:$(REPOSITORY)/%.cpp=$(build_path)/%.o}
# Special compile
${varcs:$(REPOSITORY)/%.c=$(build_path)/%.o} :
	@$(CC) $(ccflag)  -I$(REPOSITORY)/inc/hal/ -I$(REPOSITORY)/inc/os/tds2/ -c $< -g -o $@
${varccs:$(REPOSITORY)/%.cpp=$(build_path)/%.o} :
	@$(CXX) $(ccflag)  -I$(REPOSITORY)/inc/hal/ -I$(REPOSITORY)/inc/os/tds2/ -c $< -g -o $@
${varasms:$(REPOSITORY)/%.S=$(build_path)/%.o} : $(build_path)/%.o : $(REPOSITORY)/%.S
	@$(CC) $(ccflag)  -I$(REPOSITORY)/inc/hal/ -I$(REPOSITORY)/inc/os/tds2/ -c $< -g -o $@
${varcs:$(REPOSITORY)/%.c=$(build_path)/%.d} : $(build_path)/%.d : $(REPOSITORY)/%.c
	@$(call DEPENDENCY_DEDUCE,$@,$<,$(ccflag)  -I$(REPOSITORY)/inc/hal/ -I$(REPOSITORY)/inc/os/tds2/)
${varccs:$(REPOSITORY)/%.cpp=$(build_path)/%.d} : $(build_path)/%.d : $(REPOSITORY)/%.cpp
	@$(call DEPENDENCY_DEDUCE,$@,$<,$(ccflag)  -I$(REPOSITORY)/inc/hal/ -I$(REPOSITORY)/inc/os/tds2/)
${varasms:$(REPOSITORY)/%.S=$(build_path)/%.d} : $(build_path)/%.d : $(REPOSITORY)/%.S
	@$(call DEPENDENCY_DEDUCE,$@,$<,$(ccflag)-I$(REPOSITORY)/inc/hal/ -I$(REPOSITORY)/inc/os/tds2/)
else
M_HLD_OSD :
endif

.PHONY : M_HLD_PAN
ifeq ($(M_HLD_PAN),y)
varcs := \
	 $(REPOSITORY)/SRC/hld/pan//pan.c
varccs :=
varasms :=
mdfs := \
	 ${REPOSITORY}/SRC/hld/pan/hld_pan.mdf
varofiles := 
varlibs := 
SRCS += $(varcs)
SRCCS += $(varccs)
ASMS += $(varasms)
MDFS +=  $(mdfs)
OFILES += $(varofiles)
LIBS += $(varlibs)
M_HLD_PAN: ${varasms:$(REPOSITORY)/%.S=$(build_path)/%.o} ${varcs:$(REPOSITORY)/%.c=$(build_path)/%.o} ${varccs:$(REPOSITORY)/%.cpp=$(build_path)/%.o}
# Special compile
${varcs:$(REPOSITORY)/%.c=$(build_path)/%.o} :
	@$(CC) $(ccflag)  -I$(REPOSITORY)/inc/hal/ -I$(REPOSITORY)/inc/os/tds2/ -c $< -g -o $@
${varccs:$(REPOSITORY)/%.cpp=$(build_path)/%.o} :
	@$(CXX) $(ccflag)  -I$(REPOSITORY)/inc/hal/ -I$(REPOSITORY)/inc/os/tds2/ -c $< -g -o $@
${varasms:$(REPOSITORY)/%.S=$(build_path)/%.o} : $(build_path)/%.o : $(REPOSITORY)/%.S
	@$(CC) $(ccflag)  -I$(REPOSITORY)/inc/hal/ -I$(REPOSITORY)/inc/os/tds2/ -c $< -g -o $@
${varcs:$(REPOSITORY)/%.c=$(build_path)/%.d} : $(build_path)/%.d : $(REPOSITORY)/%.c
	@$(call DEPENDENCY_DEDUCE,$@,$<,$(ccflag)  -I$(REPOSITORY)/inc/hal/ -I$(REPOSITORY)/inc/os/tds2/)
${varccs:$(REPOSITORY)/%.cpp=$(build_path)/%.d} : $(build_path)/%.d : $(REPOSITORY)/%.cpp
	@$(call DEPENDENCY_DEDUCE,$@,$<,$(ccflag)  -I$(REPOSITORY)/inc/hal/ -I$(REPOSITORY)/inc/os/tds2/)
${varasms:$(REPOSITORY)/%.S=$(build_path)/%.d} : $(build_path)/%.d : $(REPOSITORY)/%.S
	@$(call DEPENDENCY_DEDUCE,$@,$<,$(ccflag)-I$(REPOSITORY)/inc/hal/ -I$(REPOSITORY)/inc/os/tds2/)
else
M_HLD_PAN :
endif

.PHONY : M_HLD_SMC
ifeq ($(M_HLD_SMC),y)
varcs := \
	 $(REPOSITORY)/SRC/hld/smc//smc.c
varccs :=
varasms :=
mdfs := \
	 ${REPOSITORY}/SRC/hld/smc/hld_smc.mdf
varofiles := 
varlibs := 
SRCS += $(varcs)
SRCCS += $(varccs)
ASMS += $(varasms)
MDFS +=  $(mdfs)
OFILES += $(varofiles)
LIBS += $(varlibs)
M_HLD_SMC: ${varasms:$(REPOSITORY)/%.S=$(build_path)/%.o} ${varcs:$(REPOSITORY)/%.c=$(build_path)/%.o} ${varccs:$(REPOSITORY)/%.cpp=$(build_path)/%.o}
# Special compile
${varcs:$(REPOSITORY)/%.c=$(build_path)/%.o} :
	@$(CC) $(ccflag)  -I$(REPOSITORY)/inc/hal/ -I$(REPOSITORY)/inc/os/tds2/ -c $< -g -o $@
${varccs:$(REPOSITORY)/%.cpp=$(build_path)/%.o} :
	@$(CXX) $(ccflag)  -I$(REPOSITORY)/inc/hal/ -I$(REPOSITORY)/inc/os/tds2/ -c $< -g -o $@
${varasms:$(REPOSITORY)/%.S=$(build_path)/%.o} : $(build_path)/%.o : $(REPOSITORY)/%.S
	@$(CC) $(ccflag)  -I$(REPOSITORY)/inc/hal/ -I$(REPOSITORY)/inc/os/tds2/ -c $< -g -o $@
${varcs:$(REPOSITORY)/%.c=$(build_path)/%.d} : $(build_path)/%.d : $(REPOSITORY)/%.c
	@$(call DEPENDENCY_DEDUCE,$@,$<,$(ccflag)  -I$(REPOSITORY)/inc/hal/ -I$(REPOSITORY)/inc/os/tds2/)
${varccs:$(REPOSITORY)/%.cpp=$(build_path)/%.d} : $(build_path)/%.d : $(REPOSITORY)/%.cpp
	@$(call DEPENDENCY_DEDUCE,$@,$<,$(ccflag)  -I$(REPOSITORY)/inc/hal/ -I$(REPOSITORY)/inc/os/tds2/)
${varasms:$(REPOSITORY)/%.S=$(build_path)/%.d} : $(build_path)/%.d : $(REPOSITORY)/%.S
	@$(call DEPENDENCY_DEDUCE,$@,$<,$(ccflag)-I$(REPOSITORY)/inc/hal/ -I$(REPOSITORY)/inc/os/tds2/)
else
M_HLD_SMC :
endif

.PHONY : M_HLD_SND
ifeq ($(M_HLD_SND),y)
varcs := \
	 $(REPOSITORY)/SRC/hld/snd//snd.c
varccs :=
varasms :=
mdfs := \
	 ${REPOSITORY}/SRC/hld/snd/hld_snd.mdf
varofiles := 
varlibs := 
SRCS += $(varcs)
SRCCS += $(varccs)
ASMS += $(varasms)
MDFS +=  $(mdfs)
OFILES += $(varofiles)
LIBS += $(varlibs)
M_HLD_SND: ${varasms:$(REPOSITORY)/%.S=$(build_path)/%.o} ${varcs:$(REPOSITORY)/%.c=$(build_path)/%.o} ${varccs:$(REPOSITORY)/%.cpp=$(build_path)/%.o}
# Special compile
${varcs:$(REPOSITORY)/%.c=$(build_path)/%.o} :
	@$(CC) $(ccflag)  -I$(REPOSITORY)/inc/hal/ -I$(REPOSITORY)/inc/os/tds2/ -c $< -g -o $@
${varccs:$(REPOSITORY)/%.cpp=$(build_path)/%.o} :
	@$(CXX) $(ccflag)  -I$(REPOSITORY)/inc/hal/ -I$(REPOSITORY)/inc/os/tds2/ -c $< -g -o $@
${varasms:$(REPOSITORY)/%.S=$(build_path)/%.o} : $(build_path)/%.o : $(REPOSITORY)/%.S
	@$(CC) $(ccflag)  -I$(REPOSITORY)/inc/hal/ -I$(REPOSITORY)/inc/os/tds2/ -c $< -g -o $@
${varcs:$(REPOSITORY)/%.c=$(build_path)/%.d} : $(build_path)/%.d : $(REPOSITORY)/%.c
	@$(call DEPENDENCY_DEDUCE,$@,$<,$(ccflag)  -I$(REPOSITORY)/inc/hal/ -I$(REPOSITORY)/inc/os/tds2/)
${varccs:$(REPOSITORY)/%.cpp=$(build_path)/%.d} : $(build_path)/%.d : $(REPOSITORY)/%.cpp
	@$(call DEPENDENCY_DEDUCE,$@,$<,$(ccflag)  -I$(REPOSITORY)/inc/hal/ -I$(REPOSITORY)/inc/os/tds2/)
${varasms:$(REPOSITORY)/%.S=$(build_path)/%.d} : $(build_path)/%.d : $(REPOSITORY)/%.S
	@$(call DEPENDENCY_DEDUCE,$@,$<,$(ccflag)-I$(REPOSITORY)/inc/hal/ -I$(REPOSITORY)/inc/os/tds2/)
else
M_HLD_SND :
endif

.PHONY : M_HLD_STO
ifeq ($(M_HLD_STO),y)
varcs := \
	 $(REPOSITORY)/SRC/hld/sto//sto.c
varccs :=
varasms :=
mdfs := \
	 ${REPOSITORY}/SRC/hld/sto/hld_sto.mdf
varofiles := 
varlibs := 
SRCS += $(varcs)
SRCCS += $(varccs)
ASMS += $(varasms)
MDFS +=  $(mdfs)
OFILES += $(varofiles)
LIBS += $(varlibs)
M_HLD_STO: ${varasms:$(REPOSITORY)/%.S=$(build_path)/%.o} ${varcs:$(REPOSITORY)/%.c=$(build_path)/%.o} ${varccs:$(REPOSITORY)/%.cpp=$(build_path)/%.o}
# Special compile
${varcs:$(REPOSITORY)/%.c=$(build_path)/%.o} :
	@$(CC) $(ccflag)  -I$(REPOSITORY)/inc/hal/ -I$(REPOSITORY)/inc/os/tds2/ -c $< -g -o $@
${varccs:$(REPOSITORY)/%.cpp=$(build_path)/%.o} :
	@$(CXX) $(ccflag)  -I$(REPOSITORY)/inc/hal/ -I$(REPOSITORY)/inc/os/tds2/ -c $< -g -o $@
${varasms:$(REPOSITORY)/%.S=$(build_path)/%.o} : $(build_path)/%.o : $(REPOSITORY)/%.S
	@$(CC) $(ccflag)  -I$(REPOSITORY)/inc/hal/ -I$(REPOSITORY)/inc/os/tds2/ -c $< -g -o $@
${varcs:$(REPOSITORY)/%.c=$(build_path)/%.d} : $(build_path)/%.d : $(REPOSITORY)/%.c
	@$(call DEPENDENCY_DEDUCE,$@,$<,$(ccflag)  -I$(REPOSITORY)/inc/hal/ -I$(REPOSITORY)/inc/os/tds2/)
${varccs:$(REPOSITORY)/%.cpp=$(build_path)/%.d} : $(build_path)/%.d : $(REPOSITORY)/%.cpp
	@$(call DEPENDENCY_DEDUCE,$@,$<,$(ccflag)  -I$(REPOSITORY)/inc/hal/ -I$(REPOSITORY)/inc/os/tds2/)
${varasms:$(REPOSITORY)/%.S=$(build_path)/%.d} : $(build_path)/%.d : $(REPOSITORY)/%.S
	@$(call DEPENDENCY_DEDUCE,$@,$<,$(ccflag)-I$(REPOSITORY)/inc/hal/ -I$(REPOSITORY)/inc/os/tds2/)
else
M_HLD_STO :
endif

.PHONY : M_HLD_VENC
ifeq ($(M_HLD_VENC),y)
varcs := \
	 $(REPOSITORY)/SRC/hld/venc//venc.c
varccs :=
varasms :=
mdfs := \
	 ${REPOSITORY}/SRC/hld/venc/hld_venc.mdf
varofiles := 
varlibs := 
SRCS += $(varcs)
SRCCS += $(varccs)
ASMS += $(varasms)
MDFS +=  $(mdfs)
OFILES += $(varofiles)
LIBS += $(varlibs)
M_HLD_VENC: ${varasms:$(REPOSITORY)/%.S=$(build_path)/%.o} ${varcs:$(REPOSITORY)/%.c=$(build_path)/%.o} ${varccs:$(REPOSITORY)/%.cpp=$(build_path)/%.o}
# Special compile
${varcs:$(REPOSITORY)/%.c=$(build_path)/%.o} :
	@$(CC) $(ccflag)  -I$(REPOSITORY)/inc/hal/ -I$(REPOSITORY)/inc/os/tds2/ -c $< -g -o $@
${varccs:$(REPOSITORY)/%.cpp=$(build_path)/%.o} :
	@$(CXX) $(ccflag)  -I$(REPOSITORY)/inc/hal/ -I$(REPOSITORY)/inc/os/tds2/ -c $< -g -o $@
${varasms:$(REPOSITORY)/%.S=$(build_path)/%.o} : $(build_path)/%.o : $(REPOSITORY)/%.S
	@$(CC) $(ccflag)  -I$(REPOSITORY)/inc/hal/ -I$(REPOSITORY)/inc/os/tds2/ -c $< -g -o $@
${varcs:$(REPOSITORY)/%.c=$(build_path)/%.d} : $(build_path)/%.d : $(REPOSITORY)/%.c
	@$(call DEPENDENCY_DEDUCE,$@,$<,$(ccflag)  -I$(REPOSITORY)/inc/hal/ -I$(REPOSITORY)/inc/os/tds2/)
${varccs:$(REPOSITORY)/%.cpp=$(build_path)/%.d} : $(build_path)/%.d : $(REPOSITORY)/%.cpp
	@$(call DEPENDENCY_DEDUCE,$@,$<,$(ccflag)  -I$(REPOSITORY)/inc/hal/ -I$(REPOSITORY)/inc/os/tds2/)
${varasms:$(REPOSITORY)/%.S=$(build_path)/%.d} : $(build_path)/%.d : $(REPOSITORY)/%.S
	@$(call DEPENDENCY_DEDUCE,$@,$<,$(ccflag)-I$(REPOSITORY)/inc/hal/ -I$(REPOSITORY)/inc/os/tds2/)
else
M_HLD_VENC :
endif

.PHONY : M_HLD_VP
ifeq ($(M_HLD_VP),y)
varcs := \
	 $(REPOSITORY)/SRC/hld/dis//vpo.c
varccs :=
varasms :=
mdfs := \
	 ${REPOSITORY}/SRC/hld/dis/hld_vp.mdf
varofiles := 
varlibs := 
SRCS += $(varcs)
SRCCS += $(varccs)
ASMS += $(varasms)
MDFS +=  $(mdfs)
OFILES += $(varofiles)
LIBS += $(varlibs)
M_HLD_VP: ${varasms:$(REPOSITORY)/%.S=$(build_path)/%.o} ${varcs:$(REPOSITORY)/%.c=$(build_path)/%.o} ${varccs:$(REPOSITORY)/%.cpp=$(build_path)/%.o}
# Special compile
${varcs:$(REPOSITORY)/%.c=$(build_path)/%.o} :
	@$(CC) $(ccflag)  -I$(REPOSITORY)/inc/hal/ -I$(REPOSITORY)/inc/os/tds2/ -c $< -g -o $@
${varccs:$(REPOSITORY)/%.cpp=$(build_path)/%.o} :
	@$(CXX) $(ccflag)  -I$(REPOSITORY)/inc/hal/ -I$(REPOSITORY)/inc/os/tds2/ -c $< -g -o $@
${varasms:$(REPOSITORY)/%.S=$(build_path)/%.o} : $(build_path)/%.o : $(REPOSITORY)/%.S
	@$(CC) $(ccflag)  -I$(REPOSITORY)/inc/hal/ -I$(REPOSITORY)/inc/os/tds2/ -c $< -g -o $@
${varcs:$(REPOSITORY)/%.c=$(build_path)/%.d} : $(build_path)/%.d : $(REPOSITORY)/%.c
	@$(call DEPENDENCY_DEDUCE,$@,$<,$(ccflag)  -I$(REPOSITORY)/inc/hal/ -I$(REPOSITORY)/inc/os/tds2/)
${varccs:$(REPOSITORY)/%.cpp=$(build_path)/%.d} : $(build_path)/%.d : $(REPOSITORY)/%.cpp
	@$(call DEPENDENCY_DEDUCE,$@,$<,$(ccflag)  -I$(REPOSITORY)/inc/hal/ -I$(REPOSITORY)/inc/os/tds2/)
${varasms:$(REPOSITORY)/%.S=$(build_path)/%.d} : $(build_path)/%.d : $(REPOSITORY)/%.S
	@$(call DEPENDENCY_DEDUCE,$@,$<,$(ccflag)-I$(REPOSITORY)/inc/hal/ -I$(REPOSITORY)/inc/os/tds2/)
else
M_HLD_VP :
endif

.PHONY : M_LIB
ifeq ($(M_LIB),y)
varcs :=
varccs :=
varasms :=
mdfs := \
	 ${REPOSITORY}/SRC/lib/lib.mdf
varofiles := 
varlibs := 
SRCS += $(varcs)
SRCCS += $(varccs)
ASMS += $(varasms)
MDFS +=  $(mdfs)
OFILES += $(varofiles)
LIBS += $(varlibs)
M_LIB: M_LIB_APPLET M_LIB_CAS M_LIB_COMMON M_LIB_DVB M_LIB_LOADER M_LIB_STORAGE M_LIB_UI
else
M_LIB :
endif

.PHONY : M_LIB_APPLET
ifeq ($(M_LIB_APPLET),y)
varcs :=
varccs :=
varasms :=
mdfs := \
	 ${REPOSITORY}/SRC/lib/libapplet/libapplet.mdf
varofiles := 
varlibs := 
SRCS += $(varcs)
SRCCS += $(varccs)
ASMS += $(varasms)
MDFS +=  $(mdfs)
OFILES += $(varofiles)
LIBS += $(varlibs)
M_LIB_APPLET: M_LIB_BOOTUPG3 M_LIB_MP M_LIB_PUB M_LIB_UPG4
else
M_LIB_APPLET :
endif

.PHONY : M_LIB_BOOTUPG3
ifeq ($(M_LIB_BOOTUPG3),y)
varcs := \
	 $(REPOSITORY)/SRC/lib/libapplet/libbootupg3//lib_ash.c \
	 $(REPOSITORY)/SRC/lib/libapplet/libbootupg3//lib_ashcmd.c \
	 $(REPOSITORY)/SRC/lib/libapplet/libbootupg3//Packet.c
varccs :=
varasms :=
mdfs := \
	 ${REPOSITORY}/SRC/lib/libapplet/libbootupg3/lib_bootupg3.mdf
varofiles := 
varlibs := 
SRCS += $(varcs)
SRCCS += $(varccs)
ASMS += $(varasms)
MDFS +=  $(mdfs)
OFILES += $(varofiles)
LIBS += $(varlibs)
M_LIB_BOOTUPG3: ${varasms:$(REPOSITORY)/%.S=$(build_path)/%.o} ${varcs:$(REPOSITORY)/%.c=$(build_path)/%.o} ${varccs:$(REPOSITORY)/%.cpp=$(build_path)/%.o}
else
M_LIB_BOOTUPG3 :
endif

.PHONY : M_LIB_MP
ifeq ($(M_LIB_MP),y)
varcs :=
varccs :=
varasms :=
mdfs := \
	 ${REPOSITORY}/SRC/lib/libapplet/libmp/lib_mp.mdf
varofiles := 
varlibs := 
SRCS += $(varcs)
SRCCS += $(varccs)
ASMS += $(varasms)
MDFS +=  $(mdfs)
OFILES += $(varofiles)
LIBS += $(varlibs)
M_LIB_MP: M_LIB_IMAGEDEC
else
M_LIB_MP :
endif

.PHONY : M_LIB_IMAGEDEC
ifeq ($(M_LIB_IMAGEDEC),y)
varcs :=
varccs :=
varasms :=
mdfs := \
	 ${REPOSITORY}/SRC/lib/libapplet/libmp/libimagedec/lib_imagedec.mdf
varofiles := 
varlibs := 
SRCS += $(varcs)
SRCCS += $(varccs)
ASMS += $(varasms)
MDFS +=  $(mdfs)
OFILES += $(varofiles)
LIBS += $(varlibs)
M_LIB_IMAGEDEC: M_LIB_GIFDEC M_LIB_JPEGDEC M_LIB_PNGDEC
else
M_LIB_IMAGEDEC :
endif

.PHONY : M_LIB_GIFDEC
ifeq ($(M_LIB_GIFDEC),y)
varcs := \
	 $(REPOSITORY)/SRC/lib/libapplet/libmp/libimagedec/gif//gif.c \
	 $(REPOSITORY)/SRC/lib/libapplet/libmp/libimagedec/gif//gif_core.c \
	 $(REPOSITORY)/SRC/lib/libapplet/libmp/libimagedec/gif//gif_in.c \
	 $(REPOSITORY)/SRC/lib/libapplet/libmp/libimagedec/gif//gif_info.c \
	 $(REPOSITORY)/SRC/lib/libapplet/libmp/libimagedec/gif//gif_list.c \
	 $(REPOSITORY)/SRC/lib/libapplet/libmp/libimagedec/gif//gif_lzw.c \
	 $(REPOSITORY)/SRC/lib/libapplet/libmp/libimagedec/gif//gif_main.c \
	 $(REPOSITORY)/SRC/lib/libapplet/libmp/libimagedec/gif//gif_out_ge.c \
	 $(REPOSITORY)/SRC/lib/libapplet/libmp/libimagedec/gif//gif_out_ge36f.c \
	 $(REPOSITORY)/SRC/lib/libapplet/libmp/libimagedec/gif//gif_out_osd.c \
	 $(REPOSITORY)/SRC/lib/libapplet/libmp/libimagedec/gif//gif_sync.c
varccs :=
varasms :=
mdfs := \
	 ${REPOSITORY}/SRC/lib/libapplet/libmp/libimagedec/gif/lib_gifdec.mdf
varofiles := 
varlibs := 
SRCS += $(varcs)
SRCCS += $(varccs)
ASMS += $(varasms)
MDFS +=  $(mdfs)
OFILES += $(varofiles)
LIBS += $(varlibs)
M_LIB_GIFDEC: ${varasms:$(REPOSITORY)/%.S=$(build_path)/%.o} ${varcs:$(REPOSITORY)/%.c=$(build_path)/%.o} ${varccs:$(REPOSITORY)/%.cpp=$(build_path)/%.o}
else
M_LIB_GIFDEC :
endif

.PHONY : M_LIB_JPEGDEC
ifeq ($(M_LIB_JPEGDEC),y)
varcs := \
	 $(REPOSITORY)/SRC/lib/libapplet/libmp/libimagedec/jpeg/m33//imagedec.c \
	 $(REPOSITORY)/SRC/lib/libapplet/libmp/libimagedec/jpeg/m33//imagedec_acc.c \
	 $(REPOSITORY)/SRC/lib/libapplet/libmp/libimagedec/jpeg/m33//imagedec_bmp_decoder.c \
	 $(REPOSITORY)/SRC/lib/libapplet/libmp/libimagedec/jpeg/m33//imagedec_covs.c \
	 $(REPOSITORY)/SRC/lib/libapplet/libmp/libimagedec/jpeg/m33//imagedec_exif.c \
	 $(REPOSITORY)/SRC/lib/libapplet/libmp/libimagedec/jpeg/m33//imagedec_idct.c \
	 $(REPOSITORY)/SRC/lib/libapplet/libmp/libimagedec/jpeg/m33//imagedec_main.c \
	 $(REPOSITORY)/SRC/lib/libapplet/libmp/libimagedec/jpeg/m33//imagedec_osd.c \
	 $(REPOSITORY)/SRC/lib/libapplet/libmp/libimagedec/jpeg/m33//imagedec_show.c \
	 $(REPOSITORY)/SRC/lib/libapplet/libmp/libimagedec/jpeg/m33//imagedec_sub.c \
	 $(REPOSITORY)/SRC/lib/libapplet/libmp/libimagedec/jpeg/m33//imagedec_trans.c \
	 $(REPOSITORY)/SRC/lib/libapplet/libmp/libimagedec/jpeg/m33//imagedec_trans_dup.c
varccs :=
varasms :=
mdfs := \
	 ${REPOSITORY}/SRC/lib/libapplet/libmp/libimagedec/jpeg/m33/lib_jpegdec.mdf
varofiles := 
varlibs := 
SRCS += $(varcs)
SRCCS += $(varccs)
ASMS += $(varasms)
MDFS +=  $(mdfs)
OFILES += $(varofiles)
LIBS += $(varlibs)
M_LIB_JPEGDEC: ${varasms:$(REPOSITORY)/%.S=$(build_path)/%.o} ${varcs:$(REPOSITORY)/%.c=$(build_path)/%.o} ${varccs:$(REPOSITORY)/%.cpp=$(build_path)/%.o}
else
M_LIB_JPEGDEC :
endif

.PHONY : M_LIB_PNGDEC
ifeq ($(M_LIB_PNGDEC),y)
varcs := \
	 $(REPOSITORY)/SRC/lib/libapplet/libmp/libimagedec/png//png.c \
	 $(REPOSITORY)/SRC/lib/libapplet/libmp/libimagedec/png//png_core.c \
	 $(REPOSITORY)/SRC/lib/libapplet/libmp/libimagedec/png//png_crc32.c \
	 $(REPOSITORY)/SRC/lib/libapplet/libmp/libimagedec/png//png_filter.c \
	 $(REPOSITORY)/SRC/lib/libapplet/libmp/libimagedec/png//png_huf_table.c \
	 $(REPOSITORY)/SRC/lib/libapplet/libmp/libimagedec/png//png_in.c \
	 $(REPOSITORY)/SRC/lib/libapplet/libmp/libimagedec/png//png_inflate.c \
	 $(REPOSITORY)/SRC/lib/libapplet/libmp/libimagedec/png//png_info.c \
	 $(REPOSITORY)/SRC/lib/libapplet/libmp/libimagedec/png//png_out.c
varccs :=
varasms :=
mdfs := \
	 ${REPOSITORY}/SRC/lib/libapplet/libmp/libimagedec/png/lib_pngdec.mdf
varofiles := 
varlibs := 
SRCS += $(varcs)
SRCCS += $(varccs)
ASMS += $(varasms)
MDFS +=  $(mdfs)
OFILES += $(varofiles)
LIBS += $(varlibs)
M_LIB_PNGDEC: ${varasms:$(REPOSITORY)/%.S=$(build_path)/%.o} ${varcs:$(REPOSITORY)/%.c=$(build_path)/%.o} ${varccs:$(REPOSITORY)/%.cpp=$(build_path)/%.o}
else
M_LIB_PNGDEC :
endif

.PHONY : M_LIB_PUB
ifeq ($(M_LIB_PUB),y)
varcs :=
varccs :=
varasms :=
mdfs := \
	 ${REPOSITORY}/SRC/lib/libapplet/libpub/lib_pub.mdf
varofiles := 
varlibs := 
SRCS += $(varcs)
SRCCS += $(varccs)
ASMS += $(varasms)
MDFS +=  $(mdfs)
OFILES += $(varofiles)
LIBS += $(varlibs)
M_LIB_PUB: M_LIB_PUB_ALL M_LIB_PUB_AS_C M_LIB_PUB_AS_ENGINE
else
M_LIB_PUB :
endif

.PHONY : M_LIB_PUB_ALL
ifeq ($(M_LIB_PUB_ALL),y)
varcs := \
	 $(REPOSITORY)/SRC/lib/libapplet/libpub/pub//lib_cc.c \
	 $(REPOSITORY)/SRC/lib/libapplet/libpub/pub//lib_device_manage.c \
	 $(REPOSITORY)/SRC/lib/libapplet/libpub/pub//lib_frontend.c \
	 $(REPOSITORY)/SRC/lib/libapplet/libpub/pub//lib_hde.c \
	 $(REPOSITORY)/SRC/lib/libapplet/libpub/pub//lib_monitor.c \
	 $(REPOSITORY)/SRC/lib/libapplet/libpub/pub//lib_multiview.c \
	 $(REPOSITORY)/SRC/lib/libapplet/libpub/pub//lib_pub.c
varccs :=
varasms :=
mdfs := \
	 ${REPOSITORY}/SRC/lib/libapplet/libpub/pub/lib_pub_all.mdf
varofiles := 
varlibs := 
SRCS += $(varcs)
SRCCS += $(varccs)
ASMS += $(varasms)
MDFS +=  $(mdfs)
OFILES += $(varofiles)
LIBS += $(varlibs)
M_LIB_PUB_ALL: ${varasms:$(REPOSITORY)/%.S=$(build_path)/%.o} ${varcs:$(REPOSITORY)/%.c=$(build_path)/%.o} ${varccs:$(REPOSITORY)/%.cpp=$(build_path)/%.o}
else
M_LIB_PUB_ALL :
endif

.PHONY : M_LIB_PUB_AS_C
ifeq ($(M_LIB_PUB_AS_C),y)
varcs := \
	 $(REPOSITORY)/SRC/lib/libapplet/libpub/as_dvbc//as_dvbc.c
varccs :=
varasms :=
mdfs := \
	 ${REPOSITORY}/SRC/lib/libapplet/libpub/as_dvbc/lib_pub_as_c.mdf
varofiles := 
varlibs := 
SRCS += $(varcs)
SRCCS += $(varccs)
ASMS += $(varasms)
MDFS +=  $(mdfs)
OFILES += $(varofiles)
LIBS += $(varlibs)
M_LIB_PUB_AS_C: ${varasms:$(REPOSITORY)/%.S=$(build_path)/%.o} ${varcs:$(REPOSITORY)/%.c=$(build_path)/%.o} ${varccs:$(REPOSITORY)/%.cpp=$(build_path)/%.o}
else
M_LIB_PUB_AS_C :
endif

.PHONY : M_LIB_PUB_AS_ENGINE
ifeq ($(M_LIB_PUB_AS_ENGINE),y)
varcs := \
	 $(REPOSITORY)/SRC/lib/libapplet/libpub/as_engine//lib_as.c
varccs :=
varasms :=
mdfs := \
	 ${REPOSITORY}/SRC/lib/libapplet/libpub/as_engine/lib_pub_as_engine.mdf
varofiles := 
varlibs := 
SRCS += $(varcs)
SRCCS += $(varccs)
ASMS += $(varasms)
MDFS +=  $(mdfs)
OFILES += $(varofiles)
LIBS += $(varlibs)
M_LIB_PUB_AS_ENGINE: ${varasms:$(REPOSITORY)/%.S=$(build_path)/%.o} ${varcs:$(REPOSITORY)/%.c=$(build_path)/%.o} ${varccs:$(REPOSITORY)/%.cpp=$(build_path)/%.o}
else
M_LIB_PUB_AS_ENGINE :
endif

.PHONY : M_LIB_UPG4
ifeq ($(M_LIB_UPG4),y)
varcs := \
	 $(REPOSITORY)/SRC/lib/libapplet/libupg4//lib_upg4_com.c
varccs :=
varasms :=
mdfs := \
	 ${REPOSITORY}/SRC/lib/libapplet/libupg4/lib_upg4.mdf
varofiles := 
varlibs := 
SRCS += $(varcs)
SRCCS += $(varccs)
ASMS += $(varasms)
MDFS +=  $(mdfs)
OFILES += $(varofiles)
LIBS += $(varlibs)
M_LIB_UPG4: M_LIB_UPG4_OTA ${varasms:$(REPOSITORY)/%.S=$(build_path)/%.o} ${varcs:$(REPOSITORY)/%.c=$(build_path)/%.o} ${varccs:$(REPOSITORY)/%.cpp=$(build_path)/%.o}
else
M_LIB_UPG4 :
endif

.PHONY : M_LIB_UPG4_OTA
ifeq ($(M_LIB_UPG4_OTA),y)
varcs := \
	 $(REPOSITORY)/SRC/lib/libapplet/libupg4/ota//lib_ota_c.c \
	 $(REPOSITORY)/SRC/lib/libapplet/libupg4/ota//upg_ota_api.c
varccs :=
varasms :=
mdfs := \
	 ${REPOSITORY}/SRC/lib/libapplet/libupg4/ota/lib_upg4_ota.mdf
varofiles := 
varlibs := 
SRCS += $(varcs)
SRCCS += $(varccs)
ASMS += $(varasms)
MDFS +=  $(mdfs)
OFILES += $(varofiles)
LIBS += $(varlibs)
M_LIB_UPG4_OTA: ${varasms:$(REPOSITORY)/%.S=$(build_path)/%.o} ${varcs:$(REPOSITORY)/%.c=$(build_path)/%.o} ${varccs:$(REPOSITORY)/%.cpp=$(build_path)/%.o}
else
M_LIB_UPG4_OTA :
endif

.PHONY : M_LIB_CAS
ifeq ($(M_LIB_CAS),y)
varcs :=
varccs :=
varasms :=
mdfs := \
	 ${REPOSITORY}/SRC/lib/libcas/libcas.mdf
varofiles := 
varlibs := 
SRCS += $(varcs)
SRCCS += $(varccs)
ASMS += $(varasms)
MDFS +=  $(mdfs)
OFILES += $(varofiles)
LIBS += $(varlibs)
M_LIB_CAS: M_CDCA M_DVTCA M_GYCA
else
M_LIB_CAS :
endif

.PHONY : M_CDCA
ifeq ($(M_CDCA),y)
varcs := \
	 $(REPOSITORY)/SRC/lib/libcas/cdca//cd_catask.c \
	 $(REPOSITORY)/SRC/lib/libcas/cdca//cd_dmx.c \
	 $(REPOSITORY)/SRC/lib/libcas/cdca//cd_nvm.c \
	 $(REPOSITORY)/SRC/lib/libcas/cdca//cd_os.c \
	 $(REPOSITORY)/SRC/lib/libcas/cdca//cd_sc.c
varccs :=
varasms :=
mdfs := \
	 ${REPOSITORY}/SRC/lib/libcas/cdca/cdca.mdf
varofiles := 
varlibs := 
SRCS += $(varcs)
SRCCS += $(varccs)
ASMS += $(varasms)
MDFS +=  $(mdfs)
OFILES += $(varofiles)
LIBS += $(varlibs)
M_CDCA: ${varasms:$(REPOSITORY)/%.S=$(build_path)/%.o} ${varcs:$(REPOSITORY)/%.c=$(build_path)/%.o} ${varccs:$(REPOSITORY)/%.cpp=$(build_path)/%.o}
else
M_CDCA :
endif

.PHONY : M_DVTCA
ifeq ($(M_DVTCA),y)
varcs :=
varccs :=
varasms :=
mdfs := \
	 ${REPOSITORY}/SRC/lib/libcas/dvt/dvtca.mdf
varofiles := 
varlibs := 
SRCS += $(varcs)
SRCCS += $(varccs)
ASMS += $(varasms)
MDFS +=  $(mdfs)
OFILES += $(varofiles)
LIBS += $(varlibs)
M_DVTCA:
else
M_DVTCA :
endif

.PHONY : M_GYCA
ifeq ($(M_GYCA),y)
varcs := \
	 $(REPOSITORY)/SRC/lib/libcas/gy//cas_gy.c \
	 $(REPOSITORY)/SRC/lib/libcas/gy//gy_catask.c \
	 $(REPOSITORY)/SRC/lib/libcas/gy//gyca_dmx.c \
	 $(REPOSITORY)/SRC/lib/libcas/gy//gyca_nvm.c \
	 $(REPOSITORY)/SRC/lib/libcas/gy//gyca_os.c \
	 $(REPOSITORY)/SRC/lib/libcas/gy//gyca_sc.c
varccs :=
varasms :=
mdfs := \
	 ${REPOSITORY}/SRC/lib/libcas/gy/gyca.mdf
varofiles := 
varlibs := 
SRCS += $(varcs)
SRCCS += $(varccs)
ASMS += $(varasms)
MDFS +=  $(mdfs)
OFILES += $(varofiles)
LIBS += $(varlibs)
M_GYCA: ${varasms:$(REPOSITORY)/%.S=$(build_path)/%.o} ${varcs:$(REPOSITORY)/%.c=$(build_path)/%.o} ${varccs:$(REPOSITORY)/%.cpp=$(build_path)/%.o}
else
M_GYCA :
endif

.PHONY : M_LIB_COMMON
ifeq ($(M_LIB_COMMON),y)
varcs :=
varccs :=
varasms :=
mdfs := \
	 ${REPOSITORY}/SRC/lib/libcommon/libcommon.mdf
varofiles := 
varlibs := 
SRCS += $(varcs)
SRCCS += $(varccs)
ASMS += $(varasms)
MDFS +=  $(mdfs)
OFILES += $(varofiles)
LIBS += $(varlibs)
M_LIB_COMMON: M_LIB_C M_LIB_CHAR M_LIB_ZIP
else
M_LIB_COMMON :
endif

.PHONY : M_LIB_C
ifeq ($(M_LIB_C),y)
varcs := \
	 $(REPOSITORY)/SRC/lib/libcommon/libc//__alloc.c \
	 $(REPOSITORY)/SRC/lib/libcommon/libc//alloc.c \
	 $(REPOSITORY)/SRC/lib/libcommon/libc//fastCRC.c \
	 $(REPOSITORY)/SRC/lib/libcommon/libc//log.c \
	 $(REPOSITORY)/SRC/lib/libcommon/libc//printf.c \
	 $(REPOSITORY)/SRC/lib/libcommon/libc//scanf.c \
	 $(REPOSITORY)/SRC/lib/libcommon/libc//string.c \
	 $(REPOSITORY)/SRC/lib/libcommon/libc//time.c
varccs :=
varasms :=
mdfs := \
	 ${REPOSITORY}/SRC/lib/libcommon/libc/lib_c.mdf
varofiles := 
varlibs := 
SRCS += $(varcs)
SRCCS += $(varccs)
ASMS += $(varasms)
MDFS +=  $(mdfs)
OFILES += $(varofiles)
LIBS += $(varlibs)
M_LIB_C: ${varasms:$(REPOSITORY)/%.S=$(build_path)/%.o} ${varcs:$(REPOSITORY)/%.c=$(build_path)/%.o} ${varccs:$(REPOSITORY)/%.cpp=$(build_path)/%.o}
else
M_LIB_C :
endif

.PHONY : M_LIB_CHAR
ifeq ($(M_LIB_CHAR),y)
varcs := \
	 $(REPOSITORY)/SRC/lib/libcommon/libchar//arabic_convert.c \
	 $(REPOSITORY)/SRC/lib/libcommon/libchar//char_map.c \
	 $(REPOSITORY)/SRC/lib/libcommon/libchar//gb_2312.c \
	 $(REPOSITORY)/SRC/lib/libcommon/libchar//iso_5937.c \
	 $(REPOSITORY)/SRC/lib/libcommon/libchar//lib_char.c \
	 $(REPOSITORY)/SRC/lib/libcommon/libchar//Thai.c
varccs :=
varasms :=
mdfs := \
	 ${REPOSITORY}/SRC/lib/libcommon/libchar/lib_char.mdf
varofiles := 
varlibs := 
SRCS += $(varcs)
SRCCS += $(varccs)
ASMS += $(varasms)
MDFS +=  $(mdfs)
OFILES += $(varofiles)
LIBS += $(varlibs)
M_LIB_CHAR: ${varasms:$(REPOSITORY)/%.S=$(build_path)/%.o} ${varcs:$(REPOSITORY)/%.c=$(build_path)/%.o} ${varccs:$(REPOSITORY)/%.cpp=$(build_path)/%.o}
else
M_LIB_CHAR :
endif

.PHONY : M_LIB_ZIP
ifeq ($(M_LIB_ZIP),y)
varcs :=
varccs :=
varasms :=
mdfs := \
	 ${REPOSITORY}/SRC/lib/libcommon/libzip/libzip.mdf
varofiles := 
varlibs := 
SRCS += $(varcs)
SRCCS += $(varccs)
ASMS += $(varasms)
MDFS +=  $(mdfs)
OFILES += $(varofiles)
LIBS += $(varlibs)
M_LIB_ZIP: M_LIB_7ZIP M_LIB_GNUZIP
else
M_LIB_ZIP :
endif

.PHONY : M_LIB_7ZIP
ifeq ($(M_LIB_7ZIP),y)
varcs := \
	 $(REPOSITORY)/SRC/lib/libcommon/libzip/lib7zip//un7zip.c
varccs :=
varasms :=
mdfs := \
	 ${REPOSITORY}/SRC/lib/libcommon/libzip/lib7zip/lib_7zip.mdf
varofiles := 
varlibs := 
SRCS += $(varcs)
SRCCS += $(varccs)
ASMS += $(varasms)
MDFS +=  $(mdfs)
OFILES += $(varofiles)
LIBS += $(varlibs)
M_LIB_7ZIP: ${varasms:$(REPOSITORY)/%.S=$(build_path)/%.o} ${varcs:$(REPOSITORY)/%.c=$(build_path)/%.o} ${varccs:$(REPOSITORY)/%.cpp=$(build_path)/%.o}
else
M_LIB_7ZIP :
endif

.PHONY : M_LIB_GNUZIP
ifeq ($(M_LIB_GNUZIP),y)
varcs := \
	 $(REPOSITORY)/SRC/lib/libcommon/libzip/gnuzip//misc.c
varccs :=
varasms :=
mdfs := \
	 ${REPOSITORY}/SRC/lib/libcommon/libzip/gnuzip/lib_gnuzip.mdf
varofiles := 
varlibs := 
SRCS += $(varcs)
SRCCS += $(varccs)
ASMS += $(varasms)
MDFS +=  $(mdfs)
OFILES += $(varofiles)
LIBS += $(varlibs)
M_LIB_GNUZIP: ${varasms:$(REPOSITORY)/%.S=$(build_path)/%.o} ${varcs:$(REPOSITORY)/%.c=$(build_path)/%.o} ${varccs:$(REPOSITORY)/%.cpp=$(build_path)/%.o}
else
M_LIB_GNUZIP :
endif

.PHONY : M_LIB_DVB
ifeq ($(M_LIB_DVB),y)
varcs :=
varccs :=
varasms :=
mdfs := \
	 ${REPOSITORY}/SRC/lib/libdvb/libdvb.mdf
varofiles := 
varlibs := 
SRCS += $(varcs)
SRCCS += $(varccs)
ASMS += $(varasms)
MDFS +=  $(mdfs)
OFILES += $(varofiles)
LIBS += $(varlibs)
M_LIB_DVB: M_LIB_DBC M_LIB_EPG M_LIB_NVOD M_LIB_SI M_LIBOTA_DVBC
else
M_LIB_DVB :
endif

.PHONY : M_LIB_DBC
ifeq ($(M_LIB_DBC),y)
varcs := \
	 $(REPOSITORY)/SRC/lib/libdvb/libdbc//lib_dbc.c
varccs :=
varasms :=
mdfs := \
	 ${REPOSITORY}/SRC/lib/libdvb/libdbc/lib_dbc.mdf
varofiles := 
varlibs := 
SRCS += $(varcs)
SRCCS += $(varccs)
ASMS += $(varasms)
MDFS +=  $(mdfs)
OFILES += $(varofiles)
LIBS += $(varlibs)
M_LIB_DBC: ${varasms:$(REPOSITORY)/%.S=$(build_path)/%.o} ${varcs:$(REPOSITORY)/%.c=$(build_path)/%.o} ${varccs:$(REPOSITORY)/%.cpp=$(build_path)/%.o}
else
M_LIB_DBC :
endif

.PHONY : M_LIB_EPG
ifeq ($(M_LIB_EPG),y)
varcs := \
	 $(REPOSITORY)/SRC/lib/libdvb/libepg//eit_parser.c \
	 $(REPOSITORY)/SRC/lib/libdvb/libepg//epg_api.c \
	 $(REPOSITORY)/SRC/lib/libdvb/libepg//epg_cache.c \
	 $(REPOSITORY)/SRC/lib/libdvb/libepg//epg_db.c \
	 $(REPOSITORY)/SRC/lib/libdvb/libepg//lib_epg.c \
	 $(REPOSITORY)/SRC/lib/libdvb/libepg//x_alloc.c
varccs :=
varasms :=
mdfs := \
	 ${REPOSITORY}/SRC/lib/libdvb/libepg/lib_epg.mdf
varofiles := 
varlibs := 
SRCS += $(varcs)
SRCCS += $(varccs)
ASMS += $(varasms)
MDFS +=  $(mdfs)
OFILES += $(varofiles)
LIBS += $(varlibs)
M_LIB_EPG: ${varasms:$(REPOSITORY)/%.S=$(build_path)/%.o} ${varcs:$(REPOSITORY)/%.c=$(build_path)/%.o} ${varccs:$(REPOSITORY)/%.cpp=$(build_path)/%.o}
else
M_LIB_EPG :
endif

.PHONY : M_LIB_NVOD
ifeq ($(M_LIB_NVOD),y)
varcs := \
	 $(REPOSITORY)/SRC/lib/libdvb/lib_nvod//lib_nvod.c
varccs :=
varasms :=
mdfs := \
	 ${REPOSITORY}/SRC/lib/libdvb/lib_nvod/libnvod.mdf
varofiles := 
varlibs := 
SRCS += $(varcs)
SRCCS += $(varccs)
ASMS += $(varasms)
MDFS +=  $(mdfs)
OFILES += $(varofiles)
LIBS += $(varlibs)
M_LIB_NVOD: ${varasms:$(REPOSITORY)/%.S=$(build_path)/%.o} ${varcs:$(REPOSITORY)/%.c=$(build_path)/%.o} ${varccs:$(REPOSITORY)/%.cpp=$(build_path)/%.o}
else
M_LIB_NVOD :
endif

.PHONY : M_LIB_SI
ifeq ($(M_LIB_SI),y)
varcs :=
varccs :=
varasms :=
mdfs := \
	 ${REPOSITORY}/SRC/lib/libdvb/libsi/libsi.mdf
varofiles := 
varlibs := 
SRCS += $(varcs)
SRCCS += $(varccs)
ASMS += $(varasms)
MDFS +=  $(mdfs)
OFILES += $(varofiles)
LIBS += $(varlibs)
M_LIB_SI: M_LIB_TSI3
else
M_LIB_SI :
endif

.PHONY : M_LIB_TSI3
ifeq ($(M_LIB_TSI3),y)
varcs :=
varccs :=
varasms :=
mdfs := \
	 ${REPOSITORY}/SRC/lib/libdvb/libsi/libtsi3/libtsi3.mdf
varofiles := 
varlibs := 
SRCS += $(varcs)
SRCCS += $(varccs)
ASMS += $(varasms)
MDFS +=  $(mdfs)
OFILES += $(varofiles)
LIBS += $(varlibs)
M_LIB_TSI3: M_LIB_TSI3_CON_PSI M_LIB_TSI3_DESCRIPTOR M_LIB_TSI3_ENGINE M_LIB_TSI3_PARSER M_LIB_TSI3_PLUGIN_EIT M_LIB_TSI3_PLUGIN_MONITOR M_LIB_TSI3_PLUGIN_SEARCH M_LIB_TSI3_PLUGIN_TDT M_LIB_TSI3_UTILITY
else
M_LIB_TSI3 :
endif

.PHONY : M_LIB_TSI3_CON_PSI
ifeq ($(M_LIB_TSI3_CON_PSI),y)
varcs := \
	 $(REPOSITORY)/SRC/lib/libdvb/libsi/libtsi3/control/psi//psi_db.c
varccs :=
varasms :=
mdfs := \
	 ${REPOSITORY}/SRC/lib/libdvb/libsi/libtsi3/control/psi/lib_tsi3_con_psi.mdf
varofiles := 
varlibs := 
SRCS += $(varcs)
SRCCS += $(varccs)
ASMS += $(varasms)
MDFS +=  $(mdfs)
OFILES += $(varofiles)
LIBS += $(varlibs)
M_LIB_TSI3_CON_PSI: ${varasms:$(REPOSITORY)/%.S=$(build_path)/%.o} ${varcs:$(REPOSITORY)/%.c=$(build_path)/%.o} ${varccs:$(REPOSITORY)/%.cpp=$(build_path)/%.o}
else
M_LIB_TSI3_CON_PSI :
endif

.PHONY : M_LIB_TSI3_DESCRIPTOR
ifeq ($(M_LIB_TSI3_DESCRIPTOR),y)
varcs := \
	 $(REPOSITORY)/SRC/lib/libdvb/libsi/libtsi3/descriptor//si_desc_cab_delivery.c \
	 $(REPOSITORY)/SRC/lib/libdvb/libsi/libtsi3/descriptor//si_desc_service_list.c
varccs :=
varasms :=
mdfs := \
	 ${REPOSITORY}/SRC/lib/libdvb/libsi/libtsi3/descriptor/lib_tsi3_descriptor.mdf
varofiles := 
varlibs := 
SRCS += $(varcs)
SRCCS += $(varccs)
ASMS += $(varasms)
MDFS +=  $(mdfs)
OFILES += $(varofiles)
LIBS += $(varlibs)
M_LIB_TSI3_DESCRIPTOR: ${varasms:$(REPOSITORY)/%.S=$(build_path)/%.o} ${varcs:$(REPOSITORY)/%.c=$(build_path)/%.o} ${varccs:$(REPOSITORY)/%.cpp=$(build_path)/%.o}
else
M_LIB_TSI3_DESCRIPTOR :
endif

.PHONY : M_LIB_TSI3_ENGINE
ifeq ($(M_LIB_TSI3_ENGINE),y)
varcs := \
	 $(REPOSITORY)/SRC/lib/libdvb/libsi/libtsi3/engine//sie.c
varccs :=
varasms :=
mdfs := \
	 ${REPOSITORY}/SRC/lib/libdvb/libsi/libtsi3/engine/lib_tsi3_engine.mdf
varofiles := 
varlibs := 
SRCS += $(varcs)
SRCCS += $(varccs)
ASMS += $(varasms)
MDFS +=  $(mdfs)
OFILES += $(varofiles)
LIBS += $(varlibs)
M_LIB_TSI3_ENGINE: ${varasms:$(REPOSITORY)/%.S=$(build_path)/%.o} ${varcs:$(REPOSITORY)/%.c=$(build_path)/%.o} ${varccs:$(REPOSITORY)/%.cpp=$(build_path)/%.o}
else
M_LIB_TSI3_ENGINE :
endif

.PHONY : M_LIB_TSI3_PARSER
ifeq ($(M_LIB_TSI3_PARSER),y)
varcs := \
	 $(REPOSITORY)/SRC/lib/libdvb/libsi/libtsi3/parser//desc_loop_parser.c \
	 $(REPOSITORY)/SRC/lib/libdvb/libsi/libtsi3/parser//psi_pat.c \
	 $(REPOSITORY)/SRC/lib/libdvb/libsi/libtsi3/parser//psi_pmt.c \
	 $(REPOSITORY)/SRC/lib/libdvb/libsi/libtsi3/parser//si_bat.c \
	 $(REPOSITORY)/SRC/lib/libdvb/libsi/libtsi3/parser//si_nit.c \
	 $(REPOSITORY)/SRC/lib/libdvb/libsi/libtsi3/parser//si_sdt.c
varccs :=
varasms :=
mdfs := \
	 ${REPOSITORY}/SRC/lib/libdvb/libsi/libtsi3/parser/lib_tsi3_parser.mdf
varofiles := 
varlibs := 
SRCS += $(varcs)
SRCCS += $(varccs)
ASMS += $(varasms)
MDFS +=  $(mdfs)
OFILES += $(varofiles)
LIBS += $(varlibs)
M_LIB_TSI3_PARSER: ${varasms:$(REPOSITORY)/%.S=$(build_path)/%.o} ${varcs:$(REPOSITORY)/%.c=$(build_path)/%.o} ${varccs:$(REPOSITORY)/%.cpp=$(build_path)/%.o}
else
M_LIB_TSI3_PARSER :
endif

.PHONY : M_LIB_TSI3_PLUGIN_EIT
ifeq ($(M_LIB_TSI3_PLUGIN_EIT),y)
varcs := \
	 $(REPOSITORY)/SRC/lib/libdvb/libsi/libtsi3/plugin/eit//sie_eit.c
varccs :=
varasms :=
mdfs := \
	 ${REPOSITORY}/SRC/lib/libdvb/libsi/libtsi3/plugin/eit/lib_tsi3_plugin_eit.mdf
varofiles := 
varlibs := 
SRCS += $(varcs)
SRCCS += $(varccs)
ASMS += $(varasms)
MDFS +=  $(mdfs)
OFILES += $(varofiles)
LIBS += $(varlibs)
M_LIB_TSI3_PLUGIN_EIT: ${varasms:$(REPOSITORY)/%.S=$(build_path)/%.o} ${varcs:$(REPOSITORY)/%.c=$(build_path)/%.o} ${varccs:$(REPOSITORY)/%.cpp=$(build_path)/%.o}
else
M_LIB_TSI3_PLUGIN_EIT :
endif

.PHONY : M_LIB_TSI3_PLUGIN_MONITOR
ifeq ($(M_LIB_TSI3_PLUGIN_MONITOR),y)
varcs := \
	 $(REPOSITORY)/SRC/lib/libdvb/libsi/libtsi3/plugin/monitor//sie_monitor_core.c \
	 $(REPOSITORY)/SRC/lib/libdvb/libsi/libtsi3/plugin/monitor//sie_monitor_dtg.c \
	 $(REPOSITORY)/SRC/lib/libdvb/libsi/libtsi3/plugin/monitor//sie_monitor_query.c \
	 $(REPOSITORY)/SRC/lib/libdvb/libsi/libtsi3/plugin/monitor//ts_monitor.c
varccs :=
varasms :=
mdfs := \
	 ${REPOSITORY}/SRC/lib/libdvb/libsi/libtsi3/plugin/monitor/lib_tsi3_plugin_monitor.mdf
varofiles := 
varlibs := 
SRCS += $(varcs)
SRCCS += $(varccs)
ASMS += $(varasms)
MDFS +=  $(mdfs)
OFILES += $(varofiles)
LIBS += $(varlibs)
M_LIB_TSI3_PLUGIN_MONITOR: ${varasms:$(REPOSITORY)/%.S=$(build_path)/%.o} ${varcs:$(REPOSITORY)/%.c=$(build_path)/%.o} ${varccs:$(REPOSITORY)/%.cpp=$(build_path)/%.o}
else
M_LIB_TSI3_PLUGIN_MONITOR :
endif

.PHONY : M_LIB_TSI3_PLUGIN_SEARCH
ifeq ($(M_LIB_TSI3_PLUGIN_SEARCH),y)
varcs := \
	 $(REPOSITORY)/SRC/lib/libdvb/libsi/libtsi3/plugin/search//si_search.c
varccs :=
varasms :=
mdfs := \
	 ${REPOSITORY}/SRC/lib/libdvb/libsi/libtsi3/plugin/search/lib_tsi3_plugin_search.mdf
varofiles := 
varlibs := 
SRCS += $(varcs)
SRCCS += $(varccs)
ASMS += $(varasms)
MDFS +=  $(mdfs)
OFILES += $(varofiles)
LIBS += $(varlibs)
M_LIB_TSI3_PLUGIN_SEARCH: ${varasms:$(REPOSITORY)/%.S=$(build_path)/%.o} ${varcs:$(REPOSITORY)/%.c=$(build_path)/%.o} ${varccs:$(REPOSITORY)/%.cpp=$(build_path)/%.o}
else
M_LIB_TSI3_PLUGIN_SEARCH :
endif

.PHONY : M_LIB_TSI3_PLUGIN_TDT
ifeq ($(M_LIB_TSI3_PLUGIN_TDT),y)
varcs := \
	 $(REPOSITORY)/SRC/lib/libdvb/libsi/libtsi3/plugin/tdt//si_tdt.c
varccs :=
varasms :=
mdfs := \
	 ${REPOSITORY}/SRC/lib/libdvb/libsi/libtsi3/plugin/tdt/lib_tsi3_plugin_tdt.mdf
varofiles := 
varlibs := 
SRCS += $(varcs)
SRCCS += $(varccs)
ASMS += $(varasms)
MDFS +=  $(mdfs)
OFILES += $(varofiles)
LIBS += $(varlibs)
M_LIB_TSI3_PLUGIN_TDT: ${varasms:$(REPOSITORY)/%.S=$(build_path)/%.o} ${varcs:$(REPOSITORY)/%.c=$(build_path)/%.o} ${varccs:$(REPOSITORY)/%.cpp=$(build_path)/%.o}
else
M_LIB_TSI3_PLUGIN_TDT :
endif

.PHONY : M_LIB_TSI3_UTILITY
ifeq ($(M_LIB_TSI3_UTILITY),y)
varcs := \
	 $(REPOSITORY)/SRC/lib/libdvb/libsi/libtsi3/utility//si_utility.c
varccs :=
varasms :=
mdfs := \
	 ${REPOSITORY}/SRC/lib/libdvb/libsi/libtsi3/utility/lib_tsi3_utility.mdf
varofiles := 
varlibs := 
SRCS += $(varcs)
SRCCS += $(varccs)
ASMS += $(varasms)
MDFS +=  $(mdfs)
OFILES += $(varofiles)
LIBS += $(varlibs)
M_LIB_TSI3_UTILITY: ${varasms:$(REPOSITORY)/%.S=$(build_path)/%.o} ${varcs:$(REPOSITORY)/%.c=$(build_path)/%.o} ${varccs:$(REPOSITORY)/%.cpp=$(build_path)/%.o}
else
M_LIB_TSI3_UTILITY :
endif

.PHONY : M_LIBOTA_DVBC
ifeq ($(M_LIBOTA_DVBC),y)
varcs := \
	 $(REPOSITORY)/SRC/lib/libdvb/libota_dvbc//lib_ota.c
varccs :=
varasms :=
mdfs := \
	 ${REPOSITORY}/SRC/lib/libdvb/libota_dvbc/libota.mdf
varofiles := 
varlibs := 
SRCS += $(varcs)
SRCCS += $(varccs)
ASMS += $(varasms)
MDFS +=  $(mdfs)
OFILES += $(varofiles)
LIBS += $(varlibs)
M_LIBOTA_DVBC: ${varasms:$(REPOSITORY)/%.S=$(build_path)/%.o} ${varcs:$(REPOSITORY)/%.c=$(build_path)/%.o} ${varccs:$(REPOSITORY)/%.cpp=$(build_path)/%.o}
else
M_LIBOTA_DVBC :
endif

.PHONY : M_LIB_LOADER
ifeq ($(M_LIB_LOADER),y)
varcs := \
	 $(REPOSITORY)/SRC/lib/libloader//ld_osd_common.c \
	 $(REPOSITORY)/SRC/lib/libloader//ld_osd_common_draw.c \
	 $(REPOSITORY)/SRC/lib/libloader//ld_osd_lib.c
varccs :=
varasms :=
mdfs := \
	 ${REPOSITORY}/SRC/lib/libloader/libloader.mdf
varofiles := 
varlibs := 
SRCS += $(varcs)
SRCCS += $(varccs)
ASMS += $(varasms)
MDFS +=  $(mdfs)
OFILES += $(varofiles)
LIBS += $(varlibs)
M_LIB_LOADER: ${varasms:$(REPOSITORY)/%.S=$(build_path)/%.o} ${varcs:$(REPOSITORY)/%.c=$(build_path)/%.o} ${varccs:$(REPOSITORY)/%.cpp=$(build_path)/%.o}
else
M_LIB_LOADER :
endif

.PHONY : M_LIB_STORAGE
ifeq ($(M_LIB_STORAGE),y)
varcs :=
varccs :=
varasms :=
mdfs := \
	 ${REPOSITORY}/SRC/lib/libstorage/libstorage.mdf
varofiles := 
varlibs := 
SRCS += $(varcs)
SRCCS += $(varccs)
ASMS += $(varasms)
MDFS +=  $(mdfs)
OFILES += $(varofiles)
LIBS += $(varlibs)
M_LIB_STORAGE: M_LIB_CHUNK M_LIB_DATABASE M_LIB_STBINFO
else
M_LIB_STORAGE :
endif

.PHONY : M_LIB_CHUNK
ifeq ($(M_LIB_CHUNK),y)
varcs := \
	 $(REPOSITORY)/SRC/lib/libstorage/libchunk//chunk.c \
	 $(REPOSITORY)/SRC/lib/libstorage/libchunk//chunk_sto.c
varccs :=
varasms :=
mdfs := \
	 ${REPOSITORY}/SRC/lib/libstorage/libchunk/lib_chunk.mdf
varofiles := 
varlibs := 
SRCS += $(varcs)
SRCCS += $(varccs)
ASMS += $(varasms)
MDFS +=  $(mdfs)
OFILES += $(varofiles)
LIBS += $(varlibs)
M_LIB_CHUNK: ${varasms:$(REPOSITORY)/%.S=$(build_path)/%.o} ${varcs:$(REPOSITORY)/%.c=$(build_path)/%.o} ${varccs:$(REPOSITORY)/%.cpp=$(build_path)/%.o}
else
M_LIB_CHUNK :
endif

.PHONY : M_LIB_DATABASE
ifeq ($(M_LIB_DATABASE),y)
varcs :=
varccs :=
varasms :=
mdfs := \
	 ${REPOSITORY}/SRC/lib/libstorage/libdb/libdb.mdf
varofiles := 
varlibs := 
SRCS += $(varcs)
SRCCS += $(varccs)
ASMS += $(varasms)
MDFS +=  $(mdfs)
OFILES += $(varofiles)
LIBS += $(varlibs)
M_LIB_DATABASE: M_LIB_DB4
else
M_LIB_DATABASE :
endif

.PHONY : M_LIB_DB4
ifeq ($(M_LIB_DB4),y)
varcs :=
varccs :=
varasms :=
mdfs := \
	 ${REPOSITORY}/SRC/lib/libstorage/libdb/libdb4/lib_db4.mdf
varofiles := 
varlibs := 
SRCS += $(varcs)
SRCCS += $(varccs)
ASMS += $(varasms)
MDFS +=  $(mdfs)
OFILES += $(varofiles)
LIBS += $(varlibs)
M_LIB_DB4: M_LIB_DB4_ENGINE M_LIB_DB4_INTERFACE M_LIB_DB4_PLUGIN_C
else
M_LIB_DB4 :
endif

.PHONY : M_LIB_DB4_ENGINE
ifeq ($(M_LIB_DB4_ENGINE),y)
varcs := \
	 $(REPOSITORY)/SRC/lib/libstorage/libdb/libdb4/src/engine//db_basic_operation.c \
	 $(REPOSITORY)/SRC/lib/libstorage/libdb/libdb4/src/engine//db_data_object.c \
	 $(REPOSITORY)/SRC/lib/libstorage/libdb/libdb4/src/engine//db_node_api.c \
	 $(REPOSITORY)/SRC/lib/libstorage/libdb/libdb4/src/engine//db_reclaim_ext.c
varccs :=
varasms :=
mdfs := \
	 ${REPOSITORY}/SRC/lib/libstorage/libdb/libdb4/src/engine/lib_db4_engine.mdf
varofiles := 
varlibs := 
SRCS += $(varcs)
SRCCS += $(varccs)
ASMS += $(varasms)
MDFS +=  $(mdfs)
OFILES += $(varofiles)
LIBS += $(varlibs)
M_LIB_DB4_ENGINE: ${varasms:$(REPOSITORY)/%.S=$(build_path)/%.o} ${varcs:$(REPOSITORY)/%.c=$(build_path)/%.o} ${varccs:$(REPOSITORY)/%.cpp=$(build_path)/%.o}
else
M_LIB_DB4_ENGINE :
endif

.PHONY : M_LIB_DB4_INTERFACE
ifeq ($(M_LIB_DB4_INTERFACE),y)
varcs := \
	 $(REPOSITORY)/SRC/lib/libstorage/libdb/libdb4/src/interface//db_interface.c \
	 $(REPOSITORY)/SRC/lib/libstorage/libdb/libdb4/src/interface//db_interface_ext.c \
	 $(REPOSITORY)/SRC/lib/libstorage/libdb/libdb4/src/interface//tmp_info.c
varccs :=
varasms :=
mdfs := \
	 ${REPOSITORY}/SRC/lib/libstorage/libdb/libdb4/src/interface/lib_db4_interface.mdf
varofiles := 
varlibs := 
SRCS += $(varcs)
SRCCS += $(varccs)
ASMS += $(varasms)
MDFS +=  $(mdfs)
OFILES += $(varofiles)
LIBS += $(varlibs)
M_LIB_DB4_INTERFACE: ${varasms:$(REPOSITORY)/%.S=$(build_path)/%.o} ${varcs:$(REPOSITORY)/%.c=$(build_path)/%.o} ${varccs:$(REPOSITORY)/%.cpp=$(build_path)/%.o}
else
M_LIB_DB4_INTERFACE :
endif

.PHONY : M_LIB_DB4_PLUGIN_C
ifeq ($(M_LIB_DB4_PLUGIN_C),y)
varcs := \
	 $(REPOSITORY)/SRC/lib/libstorage/libdb/libdb4/src/plugin//db_plugin_C.c
varccs :=
varasms :=
mdfs := \
	 ${REPOSITORY}/SRC/lib/libstorage/libdb/libdb4/src/plugin/lib_db4_plugin_c.mdf
varofiles := 
varlibs := 
SRCS += $(varcs)
SRCCS += $(varccs)
ASMS += $(varasms)
MDFS +=  $(mdfs)
OFILES += $(varofiles)
LIBS += $(varlibs)
M_LIB_DB4_PLUGIN_C: ${varasms:$(REPOSITORY)/%.S=$(build_path)/%.o} ${varcs:$(REPOSITORY)/%.c=$(build_path)/%.o} ${varccs:$(REPOSITORY)/%.cpp=$(build_path)/%.o}
else
M_LIB_DB4_PLUGIN_C :
endif

.PHONY : M_LIB_STBINFO
ifeq ($(M_LIB_STBINFO),y)
varcs := \
	 $(REPOSITORY)/SRC/lib/libstorage/libstbinfo//stb_info.c \
	 $(REPOSITORY)/SRC/lib/libstorage/libstbinfo//stb_info_data.c
varccs :=
varasms :=
mdfs := \
	 ${REPOSITORY}/SRC/lib/libstorage/libstbinfo/lib_stbinfo.mdf
varofiles := 
varlibs := 
SRCS += $(varcs)
SRCCS += $(varccs)
ASMS += $(varasms)
MDFS +=  $(mdfs)
OFILES += $(varofiles)
LIBS += $(varlibs)
M_LIB_STBINFO: ${varasms:$(REPOSITORY)/%.S=$(build_path)/%.o} ${varcs:$(REPOSITORY)/%.c=$(build_path)/%.o} ${varccs:$(REPOSITORY)/%.cpp=$(build_path)/%.o}
else
M_LIB_STBINFO :
endif

.PHONY : M_LIB_UI
ifeq ($(M_LIB_UI),y)
varcs :=
varccs :=
varasms :=
mdfs := \
	 ${REPOSITORY}/SRC/lib/libui/libui.mdf
varofiles := 
varlibs := 
SRCS += $(varcs)
SRCCS += $(varccs)
ASMS += $(varasms)
MDFS +=  $(mdfs)
OFILES += $(varofiles)
LIBS += $(varlibs)
M_LIB_UI: M_LIB_GE
else
M_LIB_UI :
endif

.PHONY : M_LIB_GE
ifeq ($(M_LIB_GE),y)
varcs := \
	 $(REPOSITORY)/SRC/lib/libui/libge//ge_draw_primitive.c \
	 $(REPOSITORY)/SRC/lib/libui/libge//ge_primitive_init.c \
	 $(REPOSITORY)/SRC/lib/libui/libge//gecmd_primitive.c \
	 $(REPOSITORY)/SRC/lib/libui/libge//gelib_effects.c \
	 $(REPOSITORY)/SRC/lib/libui/libge//gelib_test.c \
	 $(REPOSITORY)/SRC/lib/libui/libge//obj_bitmap.c \
	 $(REPOSITORY)/SRC/lib/libui/libge//obj_container.c \
	 $(REPOSITORY)/SRC/lib/libui/libge//obj_editfield.c \
	 $(REPOSITORY)/SRC/lib/libui/libge//obj_list.c \
	 $(REPOSITORY)/SRC/lib/libui/libge//obj_matrixbox.c \
	 $(REPOSITORY)/SRC/lib/libui/libge//obj_multisel.c \
	 $(REPOSITORY)/SRC/lib/libui/libge//obj_multitext.c \
	 $(REPOSITORY)/SRC/lib/libui/libge//obj_objectlist.c \
	 $(REPOSITORY)/SRC/lib/libui/libge//obj_progressbar.c \
	 $(REPOSITORY)/SRC/lib/libui/libge//obj_scrollbar.c \
	 $(REPOSITORY)/SRC/lib/libui/libge//obj_textfield.c \
	 $(REPOSITORY)/SRC/lib/libui/libge//osd_common.c \
	 $(REPOSITORY)/SRC/lib/libui/libge//osd_common_draw.c \
	 $(REPOSITORY)/SRC/lib/libui/libge//osd_lib.c \
	 $(REPOSITORY)/SRC/lib/libui/libge//osd_plugin.c \
	 $(REPOSITORY)/SRC/lib/libui/libge//osd_popup.c \
	 $(REPOSITORY)/SRC/lib/libui/libge//osd_window.c
varccs :=
varasms :=
mdfs := \
	 ${REPOSITORY}/SRC/lib/libui/libge/lib_ge.mdf
varofiles := 
varlibs := 
SRCS += $(varcs)
SRCCS += $(varccs)
ASMS += $(varasms)
MDFS +=  $(mdfs)
OFILES += $(varofiles)
LIBS += $(varlibs)
M_LIB_GE: ${varasms:$(REPOSITORY)/%.S=$(build_path)/%.o} ${varcs:$(REPOSITORY)/%.c=$(build_path)/%.o} ${varccs:$(REPOSITORY)/%.cpp=$(build_path)/%.o}
else
M_LIB_GE :
endif

.PHONY : M_LLD
ifeq ($(M_LLD),y)
varcs :=
varccs :=
varasms :=
mdfs := \
	 ${REPOSITORY}/SRC/LLD/lld.mdf
varofiles := 
varlibs := 
SRCS += $(varcs)
SRCCS += $(varccs)
ASMS += $(varasms)
MDFS +=  $(mdfs)
OFILES += $(varofiles)
LIBS += $(varlibs)
M_LLD: M_LLD_NIM M_LLD_PAN M_LLD_SMC M_LLD_STO M_LLD_VENC
else
M_LLD :
endif

.PHONY : M_LLD_NIM
ifeq ($(M_LLD_NIM),y)
varcs :=
varccs :=
varasms :=
mdfs := \
	 ${REPOSITORY}/SRC/LLD/nim/lld_nim.mdf
varofiles := 
varlibs := 
SRCS += $(varcs)
SRCCS += $(varccs)
ASMS += $(varasms)
MDFS +=  $(mdfs)
OFILES += $(varofiles)
LIBS += $(varlibs)
M_LLD_NIM: M_LLD_NIM_C
else
M_LLD_NIM :
endif

.PHONY : M_LLD_NIM_C
ifeq ($(M_LLD_NIM_C),y)
varcs := \
	 $(REPOSITORY)/SRC/LLD/nim/dvbc//MxL203RF.c \
	 $(REPOSITORY)/SRC/LLD/nim/dvbc//MxL203RF_API.c \
	 $(REPOSITORY)/SRC/LLD/nim/dvbc//MxL_User_Define.c \
	 $(REPOSITORY)/SRC/LLD/nim/dvbc//nim_s3202.c \
	 $(REPOSITORY)/SRC/LLD/nim/dvbc//tmbslNT220x.c \
	 $(REPOSITORY)/SRC/LLD/nim/dvbc//tmbslNT220xInstance.c \
	 $(REPOSITORY)/SRC/LLD/nim/dvbc//tmddNT220x.c \
	 $(REPOSITORY)/SRC/LLD/nim/dvbc//tmddNT220x_Advanced.c \
	 $(REPOSITORY)/SRC/LLD/nim/dvbc//tmddNT220xInstance.c \
	 $(REPOSITORY)/SRC/LLD/nim/dvbc//tun_alpstdac.c \
	 $(REPOSITORY)/SRC/LLD/nim/dvbc//tun_alpstdae.c \
	 $(REPOSITORY)/SRC/LLD/nim/dvbc//tun_alpstdqe.c \
	 $(REPOSITORY)/SRC/LLD/nim/dvbc//tun_cd1616lf.c \
	 $(REPOSITORY)/SRC/LLD/nim/dvbc//tun_dbcte702f1.c \
	 $(REPOSITORY)/SRC/LLD/nim/dvbc//tun_dct7044.c \
	 $(REPOSITORY)/SRC/LLD/nim/dvbc//tun_dct70701.c \
	 $(REPOSITORY)/SRC/LLD/nim/dvbc//tun_mxl203rf.c \
	 $(REPOSITORY)/SRC/LLD/nim/dvbc//tun_nt220x.c \
	 $(REPOSITORY)/SRC/LLD/nim/dvbc//tun_tdccg0x1f.c \
	 $(REPOSITORY)/SRC/LLD/nim/dvbc//tun_tdccg1x1f.c
varccs :=
varasms :=
mdfs := \
	 ${REPOSITORY}/SRC/LLD/nim/dvbc/lld_nim_c.mdf
varofiles := 
varlibs := 
SRCS += $(varcs)
SRCCS += $(varccs)
ASMS += $(varasms)
MDFS +=  $(mdfs)
OFILES += $(varofiles)
LIBS += $(varlibs)
M_LLD_NIM_C: M_LLD_TUN_C_RT820 ${varasms:$(REPOSITORY)/%.S=$(build_path)/%.o} ${varcs:$(REPOSITORY)/%.c=$(build_path)/%.o} ${varccs:$(REPOSITORY)/%.cpp=$(build_path)/%.o}
else
M_LLD_NIM_C :
endif

.PHONY : M_LLD_TUN_C_RT820
ifeq ($(M_LLD_TUN_C_RT820),y)
varcs := \
	 $(REPOSITORY)/SRC/LLD/nim/dvbc/rt820c//R820C.c \
	 $(REPOSITORY)/SRC/LLD/nim/dvbc/rt820c//tun_rt820.c
varccs :=
varasms :=
mdfs := \
	 ${REPOSITORY}/SRC/LLD/nim/dvbc/rt820c/lld_tun_rt820.mdf
varofiles := 
varlibs := 
SRCS += $(varcs)
SRCCS += $(varccs)
ASMS += $(varasms)
MDFS +=  $(mdfs)
OFILES += $(varofiles)
LIBS += $(varlibs)
M_LLD_TUN_C_RT820: ${varasms:$(REPOSITORY)/%.S=$(build_path)/%.o} ${varcs:$(REPOSITORY)/%.c=$(build_path)/%.o} ${varccs:$(REPOSITORY)/%.cpp=$(build_path)/%.o}
else
M_LLD_TUN_C_RT820 :
endif

.PHONY : M_LLD_PAN
ifeq ($(M_LLD_PAN),y)
varcs :=
varccs :=
varasms :=
mdfs := \
	 ${REPOSITORY}/SRC/LLD/pan/lld_pan.mdf
varofiles := 
varlibs := 
SRCS += $(varcs)
SRCCS += $(varccs)
ASMS += $(varasms)
MDFS +=  $(mdfs)
OFILES += $(varofiles)
LIBS += $(varlibs)
M_LLD_PAN: M_LLD_IRC M_LLD_PAN_CT1642 M_LLD_PAN_TP6X0
else
M_LLD_PAN :
endif

.PHONY : M_LLD_IRC
ifeq ($(M_LLD_IRC),y)
varcs := \
	 $(REPOSITORY)/SRC/LLD/pan/irc//irc_lab_std.c \
	 $(REPOSITORY)/SRC/LLD/pan/irc//irc_nec_std.c \
	 $(REPOSITORY)/SRC/LLD/pan/irc//irc_std.c
varccs :=
varasms :=
mdfs := \
	 ${REPOSITORY}/SRC/LLD/pan/irc/lld_irc.mdf
varofiles := 
varlibs := 
SRCS += $(varcs)
SRCCS += $(varccs)
ASMS += $(varasms)
MDFS +=  $(mdfs)
OFILES += $(varofiles)
LIBS += $(varlibs)
M_LLD_IRC: M_LLD_IRC_M63 ${varasms:$(REPOSITORY)/%.S=$(build_path)/%.o} ${varcs:$(REPOSITORY)/%.c=$(build_path)/%.o} ${varccs:$(REPOSITORY)/%.cpp=$(build_path)/%.o}
else
M_LLD_IRC :
endif

.PHONY : M_LLD_IRC_M63
ifeq ($(M_LLD_IRC_M63),y)
varcs := \
	 $(REPOSITORY)/SRC/LLD/pan/irc/m63//irc_6303.c
varccs :=
varasms :=
mdfs := \
	 ${REPOSITORY}/SRC/LLD/pan/irc/m63/lld_irc_m63.mdf
varofiles := 
varlibs := 
SRCS += $(varcs)
SRCCS += $(varccs)
ASMS += $(varasms)
MDFS +=  $(mdfs)
OFILES += $(varofiles)
LIBS += $(varlibs)
M_LLD_IRC_M63: ${varasms:$(REPOSITORY)/%.S=$(build_path)/%.o} ${varcs:$(REPOSITORY)/%.c=$(build_path)/%.o} ${varccs:$(REPOSITORY)/%.cpp=$(build_path)/%.o}
else
M_LLD_IRC_M63 :
endif

.PHONY : M_LLD_PAN_CT1642
ifeq ($(M_LLD_PAN_CT1642),y)
varcs := \
	 $(REPOSITORY)/SRC/LLD/pan/ct1642//pan_ct1642.c
varccs :=
varasms :=
mdfs := \
	 ${REPOSITORY}/SRC/LLD/pan/ct1642/lld_pan_ct1642.mdf
varofiles := 
varlibs := 
SRCS += $(varcs)
SRCCS += $(varccs)
ASMS += $(varasms)
MDFS +=  $(mdfs)
OFILES += $(varofiles)
LIBS += $(varlibs)
M_LLD_PAN_CT1642: ${varasms:$(REPOSITORY)/%.S=$(build_path)/%.o} ${varcs:$(REPOSITORY)/%.c=$(build_path)/%.o} ${varccs:$(REPOSITORY)/%.cpp=$(build_path)/%.o}
else
M_LLD_PAN_CT1642 :
endif

.PHONY : M_LLD_PAN_TP6X0
ifeq ($(M_LLD_PAN_TP6X0),y)
varcs := \
	 $(REPOSITORY)/SRC/LLD/pan/tp6x0//pan_tp6x0.c
varccs :=
varasms :=
mdfs := \
	 ${REPOSITORY}/SRC/LLD/pan/tp6x0/lld_pan_tp6x0.mdf
varofiles := 
varlibs := 
SRCS += $(varcs)
SRCCS += $(varccs)
ASMS += $(varasms)
MDFS +=  $(mdfs)
OFILES += $(varofiles)
LIBS += $(varlibs)
M_LLD_PAN_TP6X0: ${varasms:$(REPOSITORY)/%.S=$(build_path)/%.o} ${varcs:$(REPOSITORY)/%.c=$(build_path)/%.o} ${varccs:$(REPOSITORY)/%.cpp=$(build_path)/%.o}
else
M_LLD_PAN_TP6X0 :
endif

.PHONY : M_LLD_SMC
ifeq ($(M_LLD_SMC),y)
varcs :=
varccs :=
varasms :=
mdfs := \
	 ${REPOSITORY}/SRC/LLD/smc/lld_smc.mdf
varofiles := 
varlibs := 
SRCS += $(varcs)
SRCCS += $(varccs)
ASMS += $(varasms)
MDFS +=  $(mdfs)
OFILES += $(varofiles)
LIBS += $(varlibs)
M_LLD_SMC: M_LLD_SMARTCARD
else
M_LLD_SMC :
endif

.PHONY : M_LLD_SMARTCARD
ifeq ($(M_LLD_SMARTCARD),y)
varcs := \
	 $(REPOSITORY)/SRC/LLD/smc/smartcard//smartcard.c \
	 $(REPOSITORY)/SRC/LLD/smc/smartcard//smc_t1_buffer.c
varccs :=
varasms :=
mdfs := \
	 ${REPOSITORY}/SRC/LLD/smc/smartcard/lld_smartcard.mdf
varofiles := 
varlibs := 
SRCS += $(varcs)
SRCCS += $(varccs)
ASMS += $(varasms)
MDFS +=  $(mdfs)
OFILES += $(varofiles)
LIBS += $(varlibs)
M_LLD_SMARTCARD: ${varasms:$(REPOSITORY)/%.S=$(build_path)/%.o} ${varcs:$(REPOSITORY)/%.c=$(build_path)/%.o} ${varccs:$(REPOSITORY)/%.cpp=$(build_path)/%.o}
else
M_LLD_SMARTCARD :
endif

.PHONY : M_LLD_STO
ifeq ($(M_LLD_STO),y)
varcs :=
varccs :=
varasms :=
mdfs := \
	 ${REPOSITORY}/SRC/LLD/sto/lld_sto.mdf
varofiles := 
varlibs := 
SRCS += $(varcs)
SRCCS += $(varccs)
ASMS += $(varasms)
MDFS +=  $(mdfs)
OFILES += $(varofiles)
LIBS += $(varlibs)
M_LLD_STO: M_LLD_STO_FLASH
else
M_LLD_STO :
endif

.PHONY : M_LLD_STO_FLASH
ifeq ($(M_LLD_STO_FLASH),y)
varcs := \
	 $(REPOSITORY)/SRC/LLD/sto/flash//flash_data.c \
	 $(REPOSITORY)/SRC/LLD/sto/flash//flash_data_p.c \
	 $(REPOSITORY)/SRC/LLD/sto/flash//flash_data_s.c \
	 $(REPOSITORY)/SRC/LLD/sto/flash//flash_info.c \
	 $(REPOSITORY)/SRC/LLD/sto/flash//flash_info_p.c \
	 $(REPOSITORY)/SRC/LLD/sto/flash//flash_info_s.c \
	 $(REPOSITORY)/SRC/LLD/sto/flash//flash_pl.c \
	 $(REPOSITORY)/SRC/LLD/sto/flash//flash_pr.c \
	 $(REPOSITORY)/SRC/LLD/sto/flash//flash_r.c \
	 $(REPOSITORY)/SRC/LLD/sto/flash//flash_raw_pl_c.c \
	 $(REPOSITORY)/SRC/LLD/sto/flash//flash_raw_pr.c \
	 $(REPOSITORY)/SRC/LLD/sto/flash//flash_raw_sl_c.c \
	 $(REPOSITORY)/SRC/LLD/sto/flash//flash_raw_sr.c \
	 $(REPOSITORY)/SRC/LLD/sto/flash//flash_sl.c \
	 $(REPOSITORY)/SRC/LLD/sto/flash//flash_sr.c \
	 $(REPOSITORY)/SRC/LLD/sto/flash//sto_flash.c
varccs :=
varasms :=
mdfs := \
	 ${REPOSITORY}/SRC/LLD/sto/flash/lld_sto_flash.mdf
varofiles := 
varlibs := 
SRCS += $(varcs)
SRCCS += $(varccs)
ASMS += $(varasms)
MDFS +=  $(mdfs)
OFILES += $(varofiles)
LIBS += $(varlibs)
M_LLD_STO_FLASH: ${varasms:$(REPOSITORY)/%.S=$(build_path)/%.o} ${varcs:$(REPOSITORY)/%.c=$(build_path)/%.o} ${varccs:$(REPOSITORY)/%.cpp=$(build_path)/%.o}
else
M_LLD_STO_FLASH :
endif

.PHONY : M_LLD_VENC
ifeq ($(M_LLD_VENC),y)
varcs :=
varccs :=
varasms :=
mdfs := \
	 ${REPOSITORY}/SRC/LLD/venc/lld_venc.mdf
varofiles := 
varlibs := 
SRCS += $(varcs)
SRCCS += $(varccs)
ASMS += $(varasms)
MDFS +=  $(mdfs)
OFILES += $(varofiles)
LIBS += $(varlibs)
M_LLD_VENC: M_LLD_VENC_M33
else
M_LLD_VENC :
endif

.PHONY : M_LLD_VENC_M33
ifeq ($(M_LLD_VENC_M33),y)
varcs := \
	 $(REPOSITORY)/SRC/LLD/venc/m33//fdctref.c \
	 $(REPOSITORY)/SRC/LLD/venc/m33//motion.c \
	 $(REPOSITORY)/SRC/LLD/venc/m33//mpeg2enc.c \
	 $(REPOSITORY)/SRC/LLD/venc/m33//putbits.c \
	 $(REPOSITORY)/SRC/LLD/venc/m33//puthdr.c \
	 $(REPOSITORY)/SRC/LLD/venc/m33//putmpg.c \
	 $(REPOSITORY)/SRC/LLD/venc/m33//putpic.c \
	 $(REPOSITORY)/SRC/LLD/venc/m33//putseq.c \
	 $(REPOSITORY)/SRC/LLD/venc/m33//putvlc.c \
	 $(REPOSITORY)/SRC/LLD/venc/m33//quantize.c \
	 $(REPOSITORY)/SRC/LLD/venc/m33//transfrm.c \
	 $(REPOSITORY)/SRC/LLD/venc/m33//venc_m3327.c \
	 $(REPOSITORY)/SRC/LLD/venc/m33//venc_m3327_i_frm.c
varccs :=
varasms :=
mdfs := \
	 ${REPOSITORY}/SRC/LLD/venc/m33/lld_venc_m33.mdf
varofiles := 
varlibs := 
SRCS += $(varcs)
SRCCS += $(varccs)
ASMS += $(varasms)
MDFS +=  $(mdfs)
OFILES += $(varofiles)
LIBS += $(varlibs)
M_LLD_VENC_M33: ${varasms:$(REPOSITORY)/%.S=$(build_path)/%.o} ${varcs:$(REPOSITORY)/%.c=$(build_path)/%.o} ${varccs:$(REPOSITORY)/%.cpp=$(build_path)/%.o}
else
M_LLD_VENC_M33 :
endif

.PHONY : M_UDI
ifeq ($(M_UDI),y)
varcs :=
varccs :=
varasms :=
mdfs := \
	 ${REPOSITORY}/SRC/udi/udi.mdf
varofiles := 
varlibs := 
SRCS += $(varcs)
SRCCS += $(varccs)
ASMS += $(varasms)
MDFS +=  $(mdfs)
OFILES += $(varofiles)
LIBS += $(varlibs)
M_UDI: M_UDI_IPANEL
else
M_UDI :
endif

.PHONY : M_UDI_IPANEL
ifeq ($(M_UDI_IPANEL),y)
varcs := \
	 $(REPOSITORY)/SRC/udi/ipanel//ipanel_audio.c \
	 $(REPOSITORY)/SRC/udi/ipanel//ipanel_av.c \
	 $(REPOSITORY)/SRC/udi/ipanel//ipanel_base.c \
	 $(REPOSITORY)/SRC/udi/ipanel//ipanel_demux.c \
	 $(REPOSITORY)/SRC/udi/ipanel//ipanel_graphics.c \
	 $(REPOSITORY)/SRC/udi/ipanel//ipanel_media.c \
	 $(REPOSITORY)/SRC/udi/ipanel//ipanel_nvram.c \
	 $(REPOSITORY)/SRC/udi/ipanel//ipanel_osfile.c \
	 $(REPOSITORY)/SRC/udi/ipanel//ipanel_product.c \
	 $(REPOSITORY)/SRC/udi/ipanel//ipanel_socket.c \
	 $(REPOSITORY)/SRC/udi/ipanel//ipanel_sound.c \
	 $(REPOSITORY)/SRC/udi/ipanel//ipanel_tuner.c \
	 $(REPOSITORY)/SRC/udi/ipanel//ipanel_UrlNetCursor.c \
	 $(REPOSITORY)/SRC/udi/ipanel//ipanel_video.c
varccs :=
varasms :=
mdfs := \
	 ${REPOSITORY}/SRC/udi/ipanel/udi_ipanel.mdf
varofiles := 
varlibs := 
SRCS += $(varcs)
SRCCS += $(varccs)
ASMS += $(varasms)
MDFS +=  $(mdfs)
OFILES += $(varofiles)
LIBS += $(varlibs)
M_UDI_IPANEL: ${varasms:$(REPOSITORY)/%.S=$(build_path)/%.o} ${varcs:$(REPOSITORY)/%.c=$(build_path)/%.o} ${varccs:$(REPOSITORY)/%.cpp=$(build_path)/%.o}
else
M_UDI_IPANEL :
endif

.PHONY : M_VERSION_INFO
ifeq ($(M_VERSION_INFO),y)
varcs := \
	 $(REPOSITORY)/SRC/version//version.c
varccs :=
varasms :=
mdfs := \
	 ${REPOSITORY}/SRC/version/version_info.mdf
varofiles := 
varlibs := 
SRCS += $(varcs)
SRCCS += $(varccs)
ASMS += $(varasms)
MDFS +=  $(mdfs)
OFILES += $(varofiles)
LIBS += $(varlibs)
M_VERSION_INFO: ${varasms:$(REPOSITORY)/%.S=$(build_path)/%.o} ${varcs:$(REPOSITORY)/%.c=$(build_path)/%.o} ${varccs:$(REPOSITORY)/%.cpp=$(build_path)/%.o}
else
M_VERSION_INFO :
endif


# TARGET -- .o files used to archive. exclude LIBS.
OBJS = ${ASMS:$(REPOSITORY)/%.S=$(build_path)/%.o} ${SRCS:$(REPOSITORY)/%.c=$(build_path)/%.o} ${SRCCS:$(REPOSITORY)/%.cpp=$(build_path)/%.o}
TARGET = $(OFILES) $(OBJS)

# define a function which is resonsible for generate .d file for 
# .c / .S file. It takes 3 arguments: $1 is target file path name
# $2 is source file path name and $3 is ccflag
define DEPENDENCY_DEDUCE
	@echo generate $1
	@set -e;			\
	FullName=$1; 	\
	FileName=$${FullName##*/};	\
	DirName=$${FullName%%$$FileName};	\
	FileName=$${FileName%.d};	\
	[ -d $${DirName} ] || mkdir -p $${DirName};	\
	$(CC) -M $3 -w $2 | sed -e "s!$${FileName}\\.o[:]*!$${DirName}$${FileName}\\.o $1 :!g" > $1 ; \
	[ -s $1 ] || rm -f $1
endef

# Default rules
$(build_path)/%.o : $(REPOSITORY)/%.c
	@echo $<
	@$(CC) $(ccflag) -c $< -g -o $@

$(build_path)/%.o : $(REPOSITORY)/%.cpp
	@echo $<
	@$(CXX) $(ccflag) -c $< -g -o $@

	
$(build_path)/%.o : $(REPOSITORY)/%.S
	@echo $<
	@$(CC) $(ccflag) -c $< -g -o $@

$(build_path)/%.d : $(REPOSITORY)/%.S
	@$(call DEPENDENCY_DEDUCE,$@,$<,$(ccflag))

	
$(build_path)/%.d : $(REPOSITORY)/%.c
	@$(call DEPENDENCY_DEDUCE,$@,$<,$(ccflag))
$(build_path)/%.d : $(REPOSITORY)/%.cpp
	@$(call DEPENDENCY_DEDUCE,$@,$<,$(ccflag))

#MINCLUDE  make clean needn't check rule, pass MINCLUDE!=y
MINCLUDE=y
ifeq (${MINCLUDE},y)
-include $(ASMS:$(REPOSITORY)/%.S=$(build_path)/%.d) $(SRCS:$(REPOSITORY)/%.c=$(build_path)/%.d) $(SRCCS:$(REPOSITORY)/%.cpp=$(build_path)/%.d) $(ASMS_D:$(REPOSITORY)/%.S=$(build_path)/%.d) $(SRCS_D:$(REPOSITORY)/%.c=$(build_path)/%.d) $(SRCCS_D:$(REPOSITORY)/%.cpp=$(build_path)/%.d)
endif

# To extract LIBS
.PHONY : ExtractLibs
ExtractLibs :
	@rm -fr $(build_path)/tmp
	@mkdir $(build_path)/tmp
    ifneq ($(strip $(LIBS)),)
	@cd $(build_path)/tmp/\
	-$(AR) -x $(foreach var,$(LIBS),$(var))
    endif


# Prepare before build
.PHONY : prepare
prepare : 

	@[ -d $(build_path) ] || mkdir $(build_path);
	@[ -d $(release_path) ] || mkdir $(release_path);
	@[ -d $(release_path)/include ] || mkdir $(release_path)/include;
	@[ -d $(release_path)/lib ] || mkdir $(release_path)/lib


# Build library
.PHONY : all
all : prepare M_m3327 ExtractLibs
	rm -fr $(release_path)/lib/liball.a
	$(AR) rcs $(release_path)/lib/liball.a $(TARGET)
    ifneq ($(strip $(LIBS)),)
	$(AR) rcs $(release_path)/lib/liball.a $(shell echo $(build_path)/tmp/*.o)
    endif



# make dependency file
.PHONY : dependency
dependency : prepare 
	@echo make dependency file complete 


# clean
.PHONY : clean
clean : 
	rm -fr $(build_path)
	rm -fr $(release_path)/lib/liball.a


# all clean
.PHONY : allclean
allclean : clean
	rm -fr $(release_path)


# rebuild all
.PHONY : rebuildall
rebuildall : prepare clean
	make all MINCLUDE=y

.PHONY : release
release: 
	dos2unix prj_rel.sh
	echo 'REPOSITORY=$(REPOSITORY)' > Makefile.rel
	echo 'APP_ROOT_PATH=/d/3202c-2013.3.4/prj/app/demo/dvbc/jade_demo_new_geeya/mcf_data_broadcast_3202c/' >> Makefile.rel
	echo 'RELEASE_PATH=$(RELEASE_PROJECT_PATH)' >> Makefile.rel
	find $(build_path) -name *.o | xargs rm -f
	find $(build_path) -name *.d | xargs cat >>rel.tmp
	echo "mdfs.o: ${MDFS}" >> rel.tmp
	echo "others.o: /d/3202c-2013.3.4/prj/app/demo/dvbc/jade_demo_new_geeya/mcf_data_broadcast_3202c/data_broadcast_sdk_3202c.mcf /d/3202c-2013.3.4/prj/app/demo/dvbc/jade_demo_new_geeya/mcf_data_broadcast_3202c//sysconfig_ifdef.h">> rel.tmp
	sed "/[^\\\\]$$/a\\	@echo \$$<;export REPOSITORY=\$$(REPOSITORY); export APP_ROOT_PATH=\$$(APP_ROOT_PATH);export RELEASE_PATH=\$$(RELEASE_PATH); ./prj_rel.sh $$+" rel.tmp >> Makefile.rel
	-rm rel.tmp
	echo "release: $(OBJS) mdfs.o others.o" >> Makefile.rel
	echo "	@echo library release done!" >> Makefile.rel
	$(MAKE) -sf Makefile.rel release
	-rm -f Makefile.rel
	-rm -f prj_rel.sh

# End of Makefile
