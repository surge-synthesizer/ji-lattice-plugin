# Lattices: A JI MTS Source

![Exampe pic](https://github.com/Andreya-Autumn/lattices/assets/LE.png)

The code in this repo builds a microtonal control plugin which makes extended Just Intonation easy to explore on regular 12-note interfaces. A baseline was put down by BaconPaul in early 2023, which I (Andreya) have been building on in the second half of 2024. It is currently in Alpha. More on that below.


Lattices works by tuning nearby synths to Just Intonation by acting as an **MTS-ESP source** (for more info on what that means, [click here](https://surge-synthesizer.github.io/tuning-guide/#mts-esp)), and displaying a **lattice pattern** that help navigate the harmonic possibilities. 

These lattice graphics have been used (drawn in slightly different ways) by lots of folks over the years. My most direct inspiration comes from W.A. Mathieu's wonderful book [Harmonic Experience](https://www.innertraditions.com/books/harmonic-experience) (Thanks Allaudin!), and the wonderful work of [Gary Garett](https://www.youtube.com/watch?v=jA1C9VFqJKo) (thanks Gary!). 

Compared to most other MTS-ESP sources (listed in the link above), this one is very limited in scope. It has one particular purpose which it aims to excel at. Non-12 note scales and non-octave scales (while wonderful in their own way) are out of scope here. That, plus a few other specific assumptions, allows us to navigate a theoretically infinite set of pitches with a fairly simple set of controls. 

## Using

I recommend using it with a regular ordinary MIDI keyboard, plus any MIDI controller that has at least 5 buttons capable of sending momentary CC messages. I've been using a Launchpad Mini while developing it, but anything goes really. 

Load Lattices in your DAW, alongside some compatible instrument of your choice. Many are listed in the link above. If you don't have any, I recommend [Surge](https://surge-synthesizer.github.io/).

By default, Lattices will tune your synths, to a Just Intonation scale known in many places in the world under different names, but which I'm calling [Duodene]( http://www.tonalsoft.com/enc/d/duodene.aspx). 
The Lattices graphic you see represents the infinite pitch space of Just Intonation. The red sphere reading 1/1, represents the tonic (C, by default). Horizontal neighbors are a fifth apart, diagonally up-right is always a major third up, down-right a minor third up. (Octave-reduced... blablablabla, this text will be improved eventually)

You should see a pattern of twelve spheres near the middle which are lit up (while the rest are dimmed). That's the 12-note (octave-repeating) Duodene scale which currently will be sent to your synths.If you've never played in it before, you can quickly discover the (subjective) upside and downside it has over the common 12-equal tuning: Play a C major chord followed by a D major chord. Assuming everything has loaded and connected correctly, the C should sound extremely stable and the D should sound distinctly challenged. 

Send Lattices a momentary trigger message (max value for a short moment) at MIDI CC 5 on channel 1. The lit-up section on the lattice should move one step to the west. Now the D chord will sound stable! :) 

Some other chord will now sound out-of-tune instead, but that's ok! The point is, in anticipation of a sour chord, we can shift the mapping around. This way we can make perfectly in-tune music using more than 12-notes, but using common 12-note tools! This invites new ways to explore harmony. Etc etc blablabla. More to come. :) 

Other usage notes: 

-  "Origin" tab bottom right has controls to select which note is the 1/1, and which frequency it should have. (Changing root note also sets the frequency to that currently held by that note). 
-  "Midi Settings" bottom left lets you choose which CCs trigger the tuning changes. "Home" here means "return to where we startred"
- "Mode" top left requires some more explanation... will get to it eventually.

For now it only does 5-limit tuning. But the default Duodene mode will eventually will allow interdimensional visitors. :) Pythagorean mode may even allow some tempered tunings eventually! Stay tuned! 


## Lattices is currently in Alpha. 
Here's what that means in this case: 

- The overall design is pretty finished, but almost every feature is still very half-baked. 
- The process of baking them is still pretty volatile! Everything is subject to change! 
- Streaming stability is a definite maybe. This is not as dangerous with an MTS-ESP plug as with a synth (and no current features will be outright removed), but still, know that stuff will change and loading an old project with a new version may not work.
- Many planned features are missing. I will make an issue list here soon with planned features. Check that before making feature requests (feature requests are welcome though). 

This project may soon become a Surge Synth Team product (should the team consent). Otherwise will stay here. 

## Building

This works on Mac, dunno about Windows and Linux! Help appreciated.

```
git clone
git submodule update --init --recursive
cmake -Bignore/build -DCMAKE_BUILD_TYPE=RELEASE
cmake --build ignore/build
```

## Contributing

Contributions welcome! The best ways to get in touch is to look up the tuning channel in the Surge Synth Team Discord and ping Andreya there. Otherwise make an issue or PR on Github.

## Licensing

This is MIT licensed but if you use the GPL3 juce as opposed to a licensed juce to distribute it
the combined work is distributed under GPL3.

