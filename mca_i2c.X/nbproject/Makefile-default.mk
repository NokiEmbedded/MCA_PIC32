#
# Generated Makefile - do not edit!
#
# Edit the Makefile in the project folder instead (../Makefile). Each target
# has a -pre and a -post target defined where you can add customized code.
#
# This makefile implements configuration specific macros and targets.


# Include project Makefile
ifeq "${IGNORE_LOCAL}" "TRUE"
# do not include local makefile. User is passing all local related variables already
else
include Makefile
# Include makefile containing local settings
ifeq "$(wildcard nbproject/Makefile-local-default.mk)" "nbproject/Makefile-local-default.mk"
include nbproject/Makefile-local-default.mk
endif
endif

# Environment
MKDIR=gnumkdir -p
RM=rm -f 
MV=mv 
CP=cp 

# Macros
CND_CONF=default
ifeq ($(TYPE_IMAGE), DEBUG_RUN)
IMAGE_TYPE=debug
OUTPUT_SUFFIX=elf
DEBUGGABLE_SUFFIX=elf
FINAL_IMAGE=${DISTDIR}/mca_i2c.X.${IMAGE_TYPE}.${OUTPUT_SUFFIX}
else
IMAGE_TYPE=production
OUTPUT_SUFFIX=hex
DEBUGGABLE_SUFFIX=elf
FINAL_IMAGE=${DISTDIR}/mca_i2c.X.${IMAGE_TYPE}.${OUTPUT_SUFFIX}
endif

ifeq ($(COMPARE_BUILD), true)
COMPARISON_BUILD=-mafrlcsj
else
COMPARISON_BUILD=
endif

# Object Directory
OBJECTDIR=build/${CND_CONF}/${IMAGE_TYPE}

# Distribution Directory
DISTDIR=dist/${CND_CONF}/${IMAGE_TYPE}

# Source Files Quoted if spaced
SOURCEFILES_QUOTED_IF_SPACED=dacCommunication.h fpgaCommunication.h i2cCommunication.h dacCommunication.c fpgaCommunication.c i2cCommunication.c newmainXC16.c

# Object Files Quoted if spaced
OBJECTFILES_QUOTED_IF_SPACED=${OBJECTDIR}/dacCommunication.o ${OBJECTDIR}/fpgaCommunication.o ${OBJECTDIR}/i2cCommunication.o ${OBJECTDIR}/dacCommunication.o ${OBJECTDIR}/fpgaCommunication.o ${OBJECTDIR}/i2cCommunication.o ${OBJECTDIR}/newmainXC16.o
POSSIBLE_DEPFILES=${OBJECTDIR}/dacCommunication.o.d ${OBJECTDIR}/fpgaCommunication.o.d ${OBJECTDIR}/i2cCommunication.o.d ${OBJECTDIR}/dacCommunication.o.d ${OBJECTDIR}/fpgaCommunication.o.d ${OBJECTDIR}/i2cCommunication.o.d ${OBJECTDIR}/newmainXC16.o.d

# Object Files
OBJECTFILES=${OBJECTDIR}/dacCommunication.o ${OBJECTDIR}/fpgaCommunication.o ${OBJECTDIR}/i2cCommunication.o ${OBJECTDIR}/dacCommunication.o ${OBJECTDIR}/fpgaCommunication.o ${OBJECTDIR}/i2cCommunication.o ${OBJECTDIR}/newmainXC16.o

# Source Files
SOURCEFILES=dacCommunication.h fpgaCommunication.h i2cCommunication.h dacCommunication.c fpgaCommunication.c i2cCommunication.c newmainXC16.c



CFLAGS=
ASFLAGS=
LDLIBSOPTIONS=

############# Tool locations ##########################################
# If you copy a project from one host to another, the path where the  #
# compiler is installed may be different.                             #
# If you open this project with MPLAB X in the new host, this         #
# makefile will be regenerated and the paths will be corrected.       #
#######################################################################
# fixDeps replaces a bunch of sed/cat/printf statements that slow down the build
FIXDEPS=fixDeps

.build-conf:  ${BUILD_SUBPROJECTS}
ifneq ($(INFORMATION_MESSAGE), )
	@echo $(INFORMATION_MESSAGE)
endif
	${MAKE}  -f nbproject/Makefile-default.mk ${DISTDIR}/mca_i2c.X.${IMAGE_TYPE}.${OUTPUT_SUFFIX}

MP_PROCESSOR_OPTION=32MX250F128D
MP_LINKER_FILE_OPTION=,--script="dacCommunication.c"
# ------------------------------------------------------------------------------------
# Rules for buildStep: assemble
ifeq ($(TYPE_IMAGE), DEBUG_RUN)
else
endif

# ------------------------------------------------------------------------------------
# Rules for buildStep: assembleWithPreprocess
ifeq ($(TYPE_IMAGE), DEBUG_RUN)
else
endif

# ------------------------------------------------------------------------------------
# Rules for buildStep: compile
ifeq ($(TYPE_IMAGE), DEBUG_RUN)
${OBJECTDIR}/dacCommunication.o: dacCommunication.h  .generated_files/flags/default/7ef59b34b2b7605c36d4f1f45a5a75607138215e .generated_files/flags/default/da39a3ee5e6b4b0d3255bfef95601890afd80709
	@${MKDIR} "${OBJECTDIR}" 
	@${RM} ${OBJECTDIR}/dacCommunication.o.d 
	@${RM} ${OBJECTDIR}/dacCommunication.o 
	${MP_CC}  $(MP_EXTRA_CC_PRE) -g -D__DEBUG -D__MPLAB_DEBUGGER_SIMULATOR=1  -fframe-base-loclist  -x c -c -mprocessor=$(MP_PROCESSOR_OPTION)  -ffunction-sections -fdata-sections -O1 -fno-common -Werror -Wall -MP -MMD -MF "${OBJECTDIR}/dacCommunication.o.d" -o ${OBJECTDIR}/dacCommunication.o dacCommunication.h    -DXPRJ_default=$(CND_CONF)    $(COMPARISON_BUILD)  -mdfp="${DFP_DIR}"  
	
${OBJECTDIR}/fpgaCommunication.o: fpgaCommunication.h  .generated_files/flags/default/a3f21a984782eb45d2aa6ed7ed110fd7623731f3 .generated_files/flags/default/da39a3ee5e6b4b0d3255bfef95601890afd80709
	@${MKDIR} "${OBJECTDIR}" 
	@${RM} ${OBJECTDIR}/fpgaCommunication.o.d 
	@${RM} ${OBJECTDIR}/fpgaCommunication.o 
	${MP_CC}  $(MP_EXTRA_CC_PRE) -g -D__DEBUG -D__MPLAB_DEBUGGER_SIMULATOR=1  -fframe-base-loclist  -x c -c -mprocessor=$(MP_PROCESSOR_OPTION)  -ffunction-sections -fdata-sections -O1 -fno-common -Werror -Wall -MP -MMD -MF "${OBJECTDIR}/fpgaCommunication.o.d" -o ${OBJECTDIR}/fpgaCommunication.o fpgaCommunication.h    -DXPRJ_default=$(CND_CONF)    $(COMPARISON_BUILD)  -mdfp="${DFP_DIR}"  
	
${OBJECTDIR}/i2cCommunication.o: i2cCommunication.h  .generated_files/flags/default/70c34c3411e2a3b5cda4ec42c350542a7f6feac1 .generated_files/flags/default/da39a3ee5e6b4b0d3255bfef95601890afd80709
	@${MKDIR} "${OBJECTDIR}" 
	@${RM} ${OBJECTDIR}/i2cCommunication.o.d 
	@${RM} ${OBJECTDIR}/i2cCommunication.o 
	${MP_CC}  $(MP_EXTRA_CC_PRE) -g -D__DEBUG -D__MPLAB_DEBUGGER_SIMULATOR=1  -fframe-base-loclist  -x c -c -mprocessor=$(MP_PROCESSOR_OPTION)  -ffunction-sections -fdata-sections -O1 -fno-common -Werror -Wall -MP -MMD -MF "${OBJECTDIR}/i2cCommunication.o.d" -o ${OBJECTDIR}/i2cCommunication.o i2cCommunication.h    -DXPRJ_default=$(CND_CONF)    $(COMPARISON_BUILD)  -mdfp="${DFP_DIR}"  
	
${OBJECTDIR}/dacCommunication.o: dacCommunication.c  .generated_files/flags/default/9a76d25e710134b8589deb796ffa2d85c534ec4d .generated_files/flags/default/da39a3ee5e6b4b0d3255bfef95601890afd80709
	@${MKDIR} "${OBJECTDIR}" 
	@${RM} ${OBJECTDIR}/dacCommunication.o.d 
	@${RM} ${OBJECTDIR}/dacCommunication.o 
	${MP_CC}  $(MP_EXTRA_CC_PRE) -g -D__DEBUG -D__MPLAB_DEBUGGER_SIMULATOR=1  -fframe-base-loclist  -x c -c -mprocessor=$(MP_PROCESSOR_OPTION)  -ffunction-sections -fdata-sections -O1 -fno-common -Werror -Wall -MP -MMD -MF "${OBJECTDIR}/dacCommunication.o.d" -o ${OBJECTDIR}/dacCommunication.o dacCommunication.c    -DXPRJ_default=$(CND_CONF)    $(COMPARISON_BUILD)  -mdfp="${DFP_DIR}"  
	
${OBJECTDIR}/fpgaCommunication.o: fpgaCommunication.c  .generated_files/flags/default/a9ae79ce674c0e7b6b3b86b1257235ab9ccb8c31 .generated_files/flags/default/da39a3ee5e6b4b0d3255bfef95601890afd80709
	@${MKDIR} "${OBJECTDIR}" 
	@${RM} ${OBJECTDIR}/fpgaCommunication.o.d 
	@${RM} ${OBJECTDIR}/fpgaCommunication.o 
	${MP_CC}  $(MP_EXTRA_CC_PRE) -g -D__DEBUG -D__MPLAB_DEBUGGER_SIMULATOR=1  -fframe-base-loclist  -x c -c -mprocessor=$(MP_PROCESSOR_OPTION)  -ffunction-sections -fdata-sections -O1 -fno-common -Werror -Wall -MP -MMD -MF "${OBJECTDIR}/fpgaCommunication.o.d" -o ${OBJECTDIR}/fpgaCommunication.o fpgaCommunication.c    -DXPRJ_default=$(CND_CONF)    $(COMPARISON_BUILD)  -mdfp="${DFP_DIR}"  
	
${OBJECTDIR}/i2cCommunication.o: i2cCommunication.c  .generated_files/flags/default/315d5356ff494727a5f59f006b47b35ceee8b0da .generated_files/flags/default/da39a3ee5e6b4b0d3255bfef95601890afd80709
	@${MKDIR} "${OBJECTDIR}" 
	@${RM} ${OBJECTDIR}/i2cCommunication.o.d 
	@${RM} ${OBJECTDIR}/i2cCommunication.o 
	${MP_CC}  $(MP_EXTRA_CC_PRE) -g -D__DEBUG -D__MPLAB_DEBUGGER_SIMULATOR=1  -fframe-base-loclist  -x c -c -mprocessor=$(MP_PROCESSOR_OPTION)  -ffunction-sections -fdata-sections -O1 -fno-common -Werror -Wall -MP -MMD -MF "${OBJECTDIR}/i2cCommunication.o.d" -o ${OBJECTDIR}/i2cCommunication.o i2cCommunication.c    -DXPRJ_default=$(CND_CONF)    $(COMPARISON_BUILD)  -mdfp="${DFP_DIR}"  
	
${OBJECTDIR}/newmainXC16.o: newmainXC16.c  .generated_files/flags/default/1d9834be7fbf1e142302cd78e58b90273fa7b423 .generated_files/flags/default/da39a3ee5e6b4b0d3255bfef95601890afd80709
	@${MKDIR} "${OBJECTDIR}" 
	@${RM} ${OBJECTDIR}/newmainXC16.o.d 
	@${RM} ${OBJECTDIR}/newmainXC16.o 
	${MP_CC}  $(MP_EXTRA_CC_PRE) -g -D__DEBUG -D__MPLAB_DEBUGGER_SIMULATOR=1  -fframe-base-loclist  -x c -c -mprocessor=$(MP_PROCESSOR_OPTION)  -ffunction-sections -fdata-sections -O1 -fno-common -Werror -Wall -MP -MMD -MF "${OBJECTDIR}/newmainXC16.o.d" -o ${OBJECTDIR}/newmainXC16.o newmainXC16.c    -DXPRJ_default=$(CND_CONF)    $(COMPARISON_BUILD)  -mdfp="${DFP_DIR}"  
	
else
${OBJECTDIR}/dacCommunication.o: dacCommunication.h  .generated_files/flags/default/a4870d69f7678769605c51cbebbe9023510083ae .generated_files/flags/default/da39a3ee5e6b4b0d3255bfef95601890afd80709
	@${MKDIR} "${OBJECTDIR}" 
	@${RM} ${OBJECTDIR}/dacCommunication.o.d 
	@${RM} ${OBJECTDIR}/dacCommunication.o 
	${MP_CC}  $(MP_EXTRA_CC_PRE)  -g -x c -c -mprocessor=$(MP_PROCESSOR_OPTION)  -ffunction-sections -fdata-sections -O1 -fno-common -Werror -Wall -MP -MMD -MF "${OBJECTDIR}/dacCommunication.o.d" -o ${OBJECTDIR}/dacCommunication.o dacCommunication.h    -DXPRJ_default=$(CND_CONF)    $(COMPARISON_BUILD)  -mdfp="${DFP_DIR}"  
	
${OBJECTDIR}/fpgaCommunication.o: fpgaCommunication.h  .generated_files/flags/default/8717be9b5be1e38250f589c2ef76bda8ad5586c .generated_files/flags/default/da39a3ee5e6b4b0d3255bfef95601890afd80709
	@${MKDIR} "${OBJECTDIR}" 
	@${RM} ${OBJECTDIR}/fpgaCommunication.o.d 
	@${RM} ${OBJECTDIR}/fpgaCommunication.o 
	${MP_CC}  $(MP_EXTRA_CC_PRE)  -g -x c -c -mprocessor=$(MP_PROCESSOR_OPTION)  -ffunction-sections -fdata-sections -O1 -fno-common -Werror -Wall -MP -MMD -MF "${OBJECTDIR}/fpgaCommunication.o.d" -o ${OBJECTDIR}/fpgaCommunication.o fpgaCommunication.h    -DXPRJ_default=$(CND_CONF)    $(COMPARISON_BUILD)  -mdfp="${DFP_DIR}"  
	
${OBJECTDIR}/i2cCommunication.o: i2cCommunication.h  .generated_files/flags/default/13a43ed3559127d0406ef76be6d075deefc74261 .generated_files/flags/default/da39a3ee5e6b4b0d3255bfef95601890afd80709
	@${MKDIR} "${OBJECTDIR}" 
	@${RM} ${OBJECTDIR}/i2cCommunication.o.d 
	@${RM} ${OBJECTDIR}/i2cCommunication.o 
	${MP_CC}  $(MP_EXTRA_CC_PRE)  -g -x c -c -mprocessor=$(MP_PROCESSOR_OPTION)  -ffunction-sections -fdata-sections -O1 -fno-common -Werror -Wall -MP -MMD -MF "${OBJECTDIR}/i2cCommunication.o.d" -o ${OBJECTDIR}/i2cCommunication.o i2cCommunication.h    -DXPRJ_default=$(CND_CONF)    $(COMPARISON_BUILD)  -mdfp="${DFP_DIR}"  
	
${OBJECTDIR}/dacCommunication.o: dacCommunication.c  .generated_files/flags/default/b4903530dc8c5785f9f3392b44fb6f89b0434a2e .generated_files/flags/default/da39a3ee5e6b4b0d3255bfef95601890afd80709
	@${MKDIR} "${OBJECTDIR}" 
	@${RM} ${OBJECTDIR}/dacCommunication.o.d 
	@${RM} ${OBJECTDIR}/dacCommunication.o 
	${MP_CC}  $(MP_EXTRA_CC_PRE)  -g -x c -c -mprocessor=$(MP_PROCESSOR_OPTION)  -ffunction-sections -fdata-sections -O1 -fno-common -Werror -Wall -MP -MMD -MF "${OBJECTDIR}/dacCommunication.o.d" -o ${OBJECTDIR}/dacCommunication.o dacCommunication.c    -DXPRJ_default=$(CND_CONF)    $(COMPARISON_BUILD)  -mdfp="${DFP_DIR}"  
	
${OBJECTDIR}/fpgaCommunication.o: fpgaCommunication.c  .generated_files/flags/default/58484c45a9c8893b7ec2d79cded731d2dfe8680f .generated_files/flags/default/da39a3ee5e6b4b0d3255bfef95601890afd80709
	@${MKDIR} "${OBJECTDIR}" 
	@${RM} ${OBJECTDIR}/fpgaCommunication.o.d 
	@${RM} ${OBJECTDIR}/fpgaCommunication.o 
	${MP_CC}  $(MP_EXTRA_CC_PRE)  -g -x c -c -mprocessor=$(MP_PROCESSOR_OPTION)  -ffunction-sections -fdata-sections -O1 -fno-common -Werror -Wall -MP -MMD -MF "${OBJECTDIR}/fpgaCommunication.o.d" -o ${OBJECTDIR}/fpgaCommunication.o fpgaCommunication.c    -DXPRJ_default=$(CND_CONF)    $(COMPARISON_BUILD)  -mdfp="${DFP_DIR}"  
	
${OBJECTDIR}/i2cCommunication.o: i2cCommunication.c  .generated_files/flags/default/666893d4fb7545ca1d58d9abd118206e6257f126 .generated_files/flags/default/da39a3ee5e6b4b0d3255bfef95601890afd80709
	@${MKDIR} "${OBJECTDIR}" 
	@${RM} ${OBJECTDIR}/i2cCommunication.o.d 
	@${RM} ${OBJECTDIR}/i2cCommunication.o 
	${MP_CC}  $(MP_EXTRA_CC_PRE)  -g -x c -c -mprocessor=$(MP_PROCESSOR_OPTION)  -ffunction-sections -fdata-sections -O1 -fno-common -Werror -Wall -MP -MMD -MF "${OBJECTDIR}/i2cCommunication.o.d" -o ${OBJECTDIR}/i2cCommunication.o i2cCommunication.c    -DXPRJ_default=$(CND_CONF)    $(COMPARISON_BUILD)  -mdfp="${DFP_DIR}"  
	
${OBJECTDIR}/newmainXC16.o: newmainXC16.c  .generated_files/flags/default/f19c91fbdcf7dc2ed3052239c67e4381b967660c .generated_files/flags/default/da39a3ee5e6b4b0d3255bfef95601890afd80709
	@${MKDIR} "${OBJECTDIR}" 
	@${RM} ${OBJECTDIR}/newmainXC16.o.d 
	@${RM} ${OBJECTDIR}/newmainXC16.o 
	${MP_CC}  $(MP_EXTRA_CC_PRE)  -g -x c -c -mprocessor=$(MP_PROCESSOR_OPTION)  -ffunction-sections -fdata-sections -O1 -fno-common -Werror -Wall -MP -MMD -MF "${OBJECTDIR}/newmainXC16.o.d" -o ${OBJECTDIR}/newmainXC16.o newmainXC16.c    -DXPRJ_default=$(CND_CONF)    $(COMPARISON_BUILD)  -mdfp="${DFP_DIR}"  
	
endif

# ------------------------------------------------------------------------------------
# Rules for buildStep: compileCPP
ifeq ($(TYPE_IMAGE), DEBUG_RUN)
else
endif

# ------------------------------------------------------------------------------------
# Rules for buildStep: link
ifeq ($(TYPE_IMAGE), DEBUG_RUN)
${DISTDIR}/mca_i2c.X.${IMAGE_TYPE}.${OUTPUT_SUFFIX}: ${OBJECTFILES}  nbproject/Makefile-${CND_CONF}.mk    dacCommunication.c
	@${MKDIR} ${DISTDIR} 
	${MP_CC} $(MP_EXTRA_LD_PRE) -g -mdebugger -D__MPLAB_DEBUGGER_SIMULATOR=1 -mprocessor=$(MP_PROCESSOR_OPTION)  -o ${DISTDIR}/mca_i2c.X.${IMAGE_TYPE}.${OUTPUT_SUFFIX} ${OBJECTFILES_QUOTED_IF_SPACED}          -DXPRJ_default=$(CND_CONF)    $(COMPARISON_BUILD)      -Wl,--defsym=__MPLAB_BUILD=1$(MP_EXTRA_LD_POST)$(MP_LINKER_FILE_OPTION),--defsym=__MPLAB_DEBUG=1,--defsym=__DEBUG=1,-D=__DEBUG_D,--defsym=__MPLAB_DEBUGGER_SIMULATOR=1,--defsym=_min_heap_size=0,--gc-sections,--no-code-in-dinit,--no-dinit-in-serial-mem,-Map="${DISTDIR}/${PROJECTNAME}.${IMAGE_TYPE}.map",--memorysummary,${DISTDIR}/memoryfile.xml -mdfp="${DFP_DIR}"
	
else
${DISTDIR}/mca_i2c.X.${IMAGE_TYPE}.${OUTPUT_SUFFIX}: ${OBJECTFILES}  nbproject/Makefile-${CND_CONF}.mk   dacCommunication.c
	@${MKDIR} ${DISTDIR} 
	${MP_CC} $(MP_EXTRA_LD_PRE)  -mprocessor=$(MP_PROCESSOR_OPTION)  -o ${DISTDIR}/mca_i2c.X.${IMAGE_TYPE}.${DEBUGGABLE_SUFFIX} ${OBJECTFILES_QUOTED_IF_SPACED}          -DXPRJ_default=$(CND_CONF)    $(COMPARISON_BUILD)  -Wl,--defsym=__MPLAB_BUILD=1$(MP_EXTRA_LD_POST)$(MP_LINKER_FILE_OPTION),--defsym=_min_heap_size=0,--gc-sections,--no-code-in-dinit,--no-dinit-in-serial-mem,-Map="${DISTDIR}/${PROJECTNAME}.${IMAGE_TYPE}.map",--memorysummary,${DISTDIR}/memoryfile.xml -mdfp="${DFP_DIR}"
	${MP_CC_DIR}\\xc32-bin2hex ${DISTDIR}/mca_i2c.X.${IMAGE_TYPE}.${DEBUGGABLE_SUFFIX} 
endif


# Subprojects
.build-subprojects:


# Subprojects
.clean-subprojects:

# Clean Targets
.clean-conf: ${CLEAN_SUBPROJECTS}
	${RM} -r ${OBJECTDIR}
	${RM} -r ${DISTDIR}

# Enable dependency checking
.dep.inc: .depcheck-impl

DEPFILES=$(wildcard ${POSSIBLE_DEPFILES})
ifneq (${DEPFILES},)
include ${DEPFILES}
endif
