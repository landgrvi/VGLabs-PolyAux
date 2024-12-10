# Loop - send/return effects loop

<img src="Loop.png" align="right">

This expander goes to the right of a BaseLoop or a Loop. It gets the dry audio from the base, applies the gains and mutes set by the knobs for each track, and puts that on the send output ports.  
The return signal is processed according to the pan and level controls, and sent back to the base to be added to the wet audio.  
Each loop can be muted or soloed.

Inputs:
- Return input: Connect the output of your effect here. These are polyphonic, and interleaved is arranged LRLRLRLRLRLRLRLR (like MixMaster's polyphonic outputs).  
You can plug into all three ports. Signals plugged into the left port and the left channels of the interleaved port are added together, and the same with the right channels.  
If you plug a cable with more than 8 channels into a left or right port, a warning light goes on. Signals from channels above 8 on those ports are dropped.

Outputs:
- Send output: Connect these to the input of your effect. These are polyphonic, and interleaved is arranged LRLRLRLRLRLRLRLR (like MixMaster's polyphonic outputs).  
The same signals will be seen on the interleaved port as on the left and right ports; the left and right ports have only the left and right channels respectively.

Controls:
- Gain/Mute knobs: These adjust the volume per track and can be clicked to mute/unmute.
- Pan knob: Applies the menu-selected pan style to the return signal.
- Level fader: Controls the volume of the return signal.
- Mute button: Sets send and return signals to 0V, preserving channel counts.
- Solo button: Solos the loop. Every other non-soloed loop is muted.

Menus:
- Return Pan: Sets the pan style for the wet output. For more on panning, see this **[article about pan laws](https://www.cs.cmu.edu/~music/icm-online/readings/panlaws/panlaws.pdf)**.
	- Use Master (default): Uses the master pan setting on the base.
	- True Pan (L + R): Adds signal from the side you're panning away from to the side you're panning towards.  
	- Linear Attenuation: Decreases the signal that you're panning away from, and doesn't change the signal that you're panning towards.
	- 3dB Boost (constant power)
	- 4.5dB Boost (compromise)
	- 6dB Boost (linear): These last three are the pan laws described in the article linked above, normalised to a gain of 1 with the knob in the middle.
- Mono input: 
	- Copy L to R (default): If a cable is connected to a left input, with no cable connected to the corresponding right input, the right input acts as if it has the same signal as the left input,
	giving you a stereo signal where left and right are copies.  
If a cable is connected to a right input, with no cable connected to the corresponding left input, the left input has no signal.
	- Do nothing: Maintains separation of left and right.
- Themes: Pick one.



