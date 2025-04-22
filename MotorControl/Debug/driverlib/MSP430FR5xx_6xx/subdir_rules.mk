################################################################################
# Automatically-generated file. Do not edit!
################################################################################

SHELL = cmd.exe

# Each subdirectory must supply rules for building sources it contributes
driverlib/MSP430FR5xx_6xx/%.obj: ../driverlib/MSP430FR5xx_6xx/%.c $(GEN_OPTS) | $(GEN_FILES) $(GEN_MISC_FILES)
	@echo 'Building file: "$<"'
	@echo 'Invoking: MSP430 Compiler'
	"C:/ti/ccs2010/ccs/tools/compiler/ti-cgt-msp430_21.6.1.LTS/bin/cl430" -vmspx --data_model=restricted --use_hw_mpy=F5 --include_path="C:/ti/ccs2010/ccs/ccs_base/msp430/include" --include_path="C:/Users/pickl/workspace_ccstheia/MotorControl" --include_path="C:/Users/pickl/workspace_ccstheia/MotorControl/driverlib/MSP430FR5xx_6xx" --include_path="C:/ti/ccs2010/ccs/tools/compiler/ti-cgt-msp430_21.6.1.LTS/include" --advice:power="none" --advice:hw_config=all --define=_MPU_ENABLE --define=DEPRECATED --define=__MSP430FR5994__ -g --printf_support=minimal --diag_warning=225 --diag_wrap=off --display_error_number --silicon_errata=CPU21 --silicon_errata=CPU22 --silicon_errata=CPU40 --preproc_with_compile --preproc_dependency="driverlib/MSP430FR5xx_6xx/$(basename $(<F)).d_raw" --obj_directory="driverlib/MSP430FR5xx_6xx" $(GEN_OPTS__FLAG) "$<"
	@echo 'Finished building: "$<"'
	@echo ' '


