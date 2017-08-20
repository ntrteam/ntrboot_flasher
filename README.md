# ntrboot_flasher
_A tool to flash that bootrom-hax goodness to your flashcart._

For details on how ntrboot works, see the last few pages of [33.5c3](https://sciresm.github.io/33-and-a-half-c3/).

## Supported Carts
![From Left to Right: Acekard 2i HW81, Acekard 2i HW44, R4i Gold 3DS RTS, R4 3D Revolution](https://i.hentai.design/uploads/big/d3a079d193ded4659a9f962556d4bc7b.jpg)
 - Acekard 2i HW-44
 - Acekard 2i HW-81
 - R4i Gold 3DS RTS (r4ids variant)
 - R4 3D Revolution (r4idsn variant)

Note that in some rare circumstances, it may be possible for the flashing process to **brick** a counterfeit flashcart and render it permanently unusable. This is unlikely, but nevertheless only the genuine (i.e. from the original manufacturer) versions of the listed flashcarts are officially supported. There is not a good way to test for counterfeit carts, unfortunately, as they usually report the same chip ID and revision as legitimate carts. Note that you are more likely to receive a counterfeit R4 than a counterfeit AK2i. Though there is a chance that many counterfeit versions of the listed flashcarts will work fine, there is too much variance to guarantee that any counterfeit is supported. As such, use this tool at your own risk.

## Installation/Usage
Instructions are documented at [@Plailect's](https://twitter.com/plailect?lang=en) [3ds.guide](https://3ds.guide) and are heavily recommended over any alternatives.

## Compiling
To compile this, you need:
- [devkitARM r47](https://3dbrew.org/wiki/Setting_up_Development_Environment)
- [firmtool](https://github.com/TuxSH/firmtool)
- [flashcart_core](https://github.com/kitling/flashcart_core) - This is bundled as a submodule in ntrboot_flasher. When cloning the repo, you **must** use the --recursive option to grab it. Example: `git clone --recursive https://github.com/kitling/ntrboot_flasher.git`
- libelm - This is currently included (improperly) in full in this repository and requires no special instructions, it will be downloaded when copying.

## Planned support/Community involvement
While we initially planned to support more carts on release, we have not been able to expand beyond the current set of supported carts due to a lack of time/energy/manpower. If you have a cart you would like supported and have the know-how for debugging/reverse engineering a flashcart, please come find one of the devs in [#Cakey on freenode](https://kiwiirc.com/client/irc.freenode.net/?nick=ntrbootuser_?#Cakey). Ideally, this toolset is extensible towards any updatable Nintendo DS/DSi flashcart. See [flashcart_core](https://github.com/kitling/flashcart_core) for more flashcart details. **Do not submit a "feature request" on this repository asking "When will <my cart\> be supported?" The answer to this question is "We don't know". See [here](https://github.com/kitling/ntrboot_flasher/issues/3#issuecomment-322000616) for more details."**

## Credits
[@Normmatt](https://github.com/Normmatt) for initial implementation, bug squashing, expertise... etc.  
[@SciresM](https://twitter.com/SciresM) for sighax/boot9strap and flashcart RE.  
[@hedgeberg](https://twitter.com/hedgeberg) for testing and flashcart RE.  
[@stuckpixel](https://twitter.com/pixel_stuck) for testing.  
[@Myria](https://twitter.com/Myriachan) for testing.  
[@Hikari](https://twitter.com/yuukishiroko) for testing.

Huge props to [@d3m3vilurr](https://twitter.com/d3m3vilurr) for figuring this out independently!
