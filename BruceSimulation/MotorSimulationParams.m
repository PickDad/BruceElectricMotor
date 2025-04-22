% Simulation Parameters
Tstep = 5e-6; %Not used currentl
BL = 0; %1=Blocked Gating, 0=Gating
Tramp = 1; %Voltage frequency ramp rate [seconds to full speed]
Vsource = 10; %DC Voltage source
Rline = 1e-3;

%Motor Parameters
Pole_Pairs = 1;
Rated_Speed = 2700; %Rated RPM
Wrated = Rated_Speed/60*2*pi; %Mechanical Rated Angular Velocity [rads/sec]
Frated = Wrated*Pole_Pairs/(2*pi); %Rated Electrical Frequency
Rs = 312e-3; %[Ohms] 312 mOhms Measured L-L Stator Resistance
Lq = 19e-6/2; %[H] 19uH Measured L-L Stator inductance as measured, refer to motor model help for equation, Lq = Lab/2
Ld = Lq; %Equal because of round rotor, not true if salient. Our rotor is round
lambda = 4.174e-4;%2e-4; Flux Linkage [Vs] 50 turns
Kv = 0.025; %Measured peak L-L Voltage at ~1000 RPM, refer to motor model help for more info, this might change to flux linkage
J_PMSM = 1.879e-6; %Moment of inertia (ask Matthew for calculations if you want)
F_PMSM = 1e-3; %Viscous friction (Best guest for our motor)
B_PMSM = 0; %Static friction coefficient
Load_Torque = 0; 
Ref_Torque = 1e-5;

%Motor Driver Parameters
Ron = 8e-3;
Vds = 0.8;
fsw = 10e3; %PWM Switching Frequency

%Control Parameters
%These still need tuning and not used in open loop simulation
Speed_Ref = Wrated;
Speed_P = 100;
Speed_I = 0;
Speed_D = 0;

Id_ref = 0;
Id_P = 10;
Id_I = 0;
Id_D = 0;

Iq_P = 100;
Iq_I = 0;
Iq_D = 0;