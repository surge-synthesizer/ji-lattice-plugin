# Lattices: A JI MTS Source

The code in this repo builds a microtonal control plugin which makes extended Just Intonation easy to explore on regular 12-note interfaces. A baseline was put down by BaconPaul in early 2023, which I (Andreya) have been periodically working on during 2024 and 2025. While it's fairly feature complete, I have chosen to keep it marked Alpha. More on that below.

![A screenshot of the plugin](resources/LatticesSegmentScreenshot.png)

Lattices works by tuning nearby synths to Just Intonation by acting as an **MTS-ESP source** (for more info on what that means, [click here](https://surge-synthesizer.github.io/tuning-guide/#mts-esp)), and displaying a **lattice pattern** that helps navigate the harmonic possibilities. 

These lattice graphics have been used (drawn in slightly different ways) by lots of folks over the years. My most direct inspiration comes from W.A. Mathieu's wonderful book [Harmonic Experience](https://www.innertraditions.com/books/harmonic-experience) (Thanks Allaudin!), and the wonderful work of [Gary Garett](https://www.youtube.com/watch?v=jA1C9VFqJKo) (thanks Gary!). 

Compared to most other MTS-ESP sources (listed in the link above), this one is very limited in scope. It has one particular purpose which it aims to excel at. Non-12-note scales and non-octave scales (while wonderful in their own way) are out of scope here. That, plus a few other specific assumptions, allows us to navigate a theoretically infinite set of pitches with a fairly simple set of controls. 

## Using

I recommend using it with a regular ordinary MIDI keyboard, plus any MIDI controller that has at least 5 buttons capable of sending momentary CC messages. I've been using a Launchpad Mini while developing it, but anything goes really. 

Load Lattices in your DAW, alongside some compatible instrument of your choice. Many are listed in the link above. If you don't have any, I recommend [Surge](https://surge-synthesizer.github.io/).

By default, Lattices will tune your synths to a Just Intonation scale known in many places in the world under different names, but which I'm calling [Duodene]( http://www.tonalsoft.com/enc/d/duodene.aspx). 
The graphic you see represents the infinite pitch space of (5-limit) Just Intonation. The red sphere reading 1/1, represents the tonic (C, by default). Horizontal neighbors are a fifth apart, diagonally up-right is always a major third up, down-right a minor third up. (Octave-reduced... etc etc etc, this text will be improved eventually, the lack of documentation is one reason it's still Alpha)

You should see a pattern of twelve spheres near the middle which are lit up (while the rest are dimmed). That's the 12-note (octave-repeating) Duodene scale which currently will be sent to your synths. If you've never played in it before, you can quickly discover the (subjective) upside and downside it has over the common 12-equal tuning: Play a C major chord followed by a Bb major chord. Assuming everything has loaded and connected correctly, the C should sound extremely stable and the Bb should sound distinctly challenged. 

Send Lattices a momentary trigger message (max value for a short moment) at MIDI CC 17 on channel 1. The lit-up section on the lattice should move one step to the east. Now the Bb chord will sound stable! :) 

Some other chord will now sound out-of-tune instead, but that's ok! The point is, in anticipation of a sour chord, we can shift the mapping around. This way we can make perfectly in-tune 5-limit music, in any key we want, modulating to any key we want. Even though this requires more than 12 notes per octave, we will be able to do this easily using common 12-note tools. This invites new ways to explore harmony. Etc etc blablabla. More to come. :) 

Other usage notes: 

-  The Settings tab at the top has controls to select which note is the 1/1, and which frequency it should have. (Changing root note also sets the frequency to that currently held by that note). 
-   It also has a mode switch which requires some more explanation... will get to it eventually.
-   The "Home CC" field (5 by default) lets you choose which CCs trigger the tuning changes. "Home" here means "return to where we started". The next 4 CCs after the one you chose (6-9 by default) will step west, east, north and south respectively. The next ones after that activate/deactivate the "visitors".
-   "Visitors"? It is a feature which lets you temporarily invite higher-limit intervals onto the 5-limit 2d lattice (and to the keyboard). You define groups of such visitors in the menu top left. Then you use the aforementioned MIDI CCs to invite/uninvite them. 
-   The MIDI CC control works well for live playing. But for a DAW arrangement it's inconvenient cause it won't recall the tuning when you skip around the timeline. For that purpose the lattice position and visitor status are exposed as parameters. I typically experiment using the CCs and when I have an idea, record/program the shifts as automation.


## Lattices is currently in Alpha. 
Here's what that means in this case: 

- The overall design is pretty finished, but some features may change somewhat before it's finished. 
- We'll try to keep it "streaming stable", meaning loading old projects with a new version should still give the same results. But I don't dare promise total success.  This is not as dangerous with an MTS-ESP plug as with a synth, but still, if you use it to make something lovely, make notes of the settings you used so you can recreate them in case (god forbid) I break something. 
- Some planned features are missing. The issue list here has the planned features mostly sketched out. Check that before making feature requests (feature requests are welcome though).
- And yeah, it needs a real user manual of course.

Other than that it's already really useful, and I'm happy to receive feature requests or bug reports if you try it!


## Building

This has been tested to work on Mac, Windows and Linux. If you just wanna use it, simply grab a nightly build from the releases link on the right.
If you wanna build it from source, you'll need git and cmake installed, possibly other things too (lemme know if I've missed something). 
Then in your CLI tool, navigate to a fitting folder and run this: 

```
git clone https://github.com/surge-synthesizer/ji-lattice-plugin.git
cd ji-lattice-plugin
git submodule update --init --recursive
cmake -Bignore/build -DCMAKE_BUILD_TYPE=RELEASE
cmake --build ignore/build
```
You should get a build in yourFolder/ji-lattice-plugin/ignore/build/lattices-artifacts or something like that. 

## Contributing

Contributions welcome! The best ways to get in touch is to look up the tuning channel in the Surge Synth Team Discord and ping Andreya there. Otherwise make an issue or PR on Github.

## Licensing

This is MIT licensed but if you use the GPL3 juce as opposed to a licensed juce to distribute it
the combined work is distributed under GPL3.

