# BaseLoop - the base module

<img src="BaseLoop.png" align="right">

This is the core module that takes in dry audio and puts out wet audio, and that everything else hooks onto. It also contains one effects loop, documented at **[Loop](loop.md)**.  
It can have an Outs expander on its left, and Loop expanders on its right.

Inputs:
- Dry input: This gets audio into the system. These are polyphonic, and interleaved is arranged LRLRLRLRLRLRLRLR (like MixMaster's polyphonic outputs).  
You can plug into all three ports. Signals plugged into the left port and the left channels of the interleaved port are added together, and the same with the right channels.  
If you plug a cable with more than 8 channels into a left or right port, a warning light goes on. Signals from channels above 8 on those ports are dropped.

Outputs:
- Wet output: This is the audio after all processing. The same signals will be seen on the interleaved port as on the left and right ports; the left and right ports have only the left and right channels respectively.

Controls:
- Pan knob: Applies the menu-selected pan style to the wet output.
- Level fader: Controls the volume of the wet output.
- Mute button: Mutes the wet output.
- Mix knob: Mixes the dry input into the wet output. All the way right gives you wet signal only, and all the way left gives you dry signal only.  
If you want finer control of wet and dry mix, add a Loop in your system with left send patched to left return and right send patched to right return, and treat that as you would any other effect.
- Track labels: Editable text fields, limited to four characters.

Menus:
- Master Pan: Sets the pan style for the wet output. For more on panning, see **[Panning info](https://www.cs.cmu.edu/~music/icm-online/readings/panlaws/panlaws.pdf)**.
	- True Pan (L + R): As you pan left, adds right signal to the left channel and decreases it in the right channel.  
	As you pan right, adds left signal to the right channel and decreases it in the left channel.
	- Linear Attenuation: I've also seen this referred to as "stereo cut". Decreases the signal that you're panning away from, and doesn't change the signal that you're panning towards.
	- 3dB Boost (constant power)
	- 4.5dB Boost (compromise, default)
	- 6dB Boost (linear): These are the pan laws described in the article linked above, normalised to a gain of 1 with the knob in the middle.
- Return Pan: Same as above, but with the option to use the Master Pan setting.
- Mono Input: 
	- Copy L to R (default): If a cable is connected to a left input, with no cable connected to the corresponding right input, the right input acts as if it has the same signal as the left input,
	giving you a stereo signal where left and right are copies.  
If a cable is connected to a right input, with no cable connected to the corresponding left input, the left input has no signal.
	- Do nothing: Maintains separation of left and right.
- Themes: Pick one.
