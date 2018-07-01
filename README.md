# ntrboot_flasher
_A tool to flash that bootrom-hax goodness to your flashcart._

For details on how ntrboot works, see the last few pages of [33.5c3](https://sciresm.github.io/33-and-a-half-c3/).

## Development has ceased

ntrboot_flasher is no longer in active development. We consider it mostly complete. Support for flashcarts that are not already supported is extremely unlikely.

[v0.4.0](https://github.com/ntrteam/ntrboot_flasher/releases/tag/v0.4.0) is the most up-to-date release. Although there are newer commits, they do not affect functionality (they fix build issues with newer devkitARM releases).

There are no known serious issues. If there are any new issues discovered, we may fix them and issue (a) new release(s).

## Supported Carts
![From Left to Right: Acekard 2i HW81, Acekard 2i HW44, R4i Gold 3DS RTS, R4i Gold 3DS, R4i Ultra, R4 3D Revolution, DSTT, R4i-SDHC RTS Lite, R4i-SDHC Dual-Core, R4-SDHC Gold Pro, R4i 3DS RTS, Infinity 3 R4i, R4i Gold 3DS Deluxe Edition, R4i-B9S](https://i.lolis.stream/uploads/big/6c535398c6a74d580ffc9092c32d7687.png)

- Acekard 2i HW-44
- Acekard 2i HW-81
- DSTT (**some flash chips only!**)
- Infinity 3 R4i (r4infinity.com)
- R4 3D Revolution (r4idsn.com)
- R4i 3DS RTS (r4i-sdhc.com)
- R4i Gold 3DS (RTS, revisions A5/A6/A7) (r4ids.cn)
- R4i Gold 3DS Deluxe Edition (r4ids.cn) (**variants of this such as 3dslink, Orange 3DS, etc. may work as well, but have not been tested!**)
- R4i Ultra (r4ultra.com)
- R4i-B9S (r4i-sdhc.com)
- R4i-SDHC Dual-Core (r4isdhc.com)
- R4i-SDHC Gold Pro (r4isdhc.com)
- R4i-SDHC RTS Lite (r4isdhc.com)

**Note:** Flashcarts from r4isdhc.com tend to have yearly re-releases; all versions of these carts (2014-2017) should work but not all have been tested.

Note that in some rare circumstances, it may be possible for the flashing process to **brick** a counterfeit flashcart and render it permanently unusable. This is unlikely, but nevertheless only the genuine (i.e. from the original manufacturer) versions of the listed flashcarts are officially supported. There is not a good way to test for counterfeit carts, unfortunately, as they usually report the same chip ID and revision as legitimate carts. Note that you are more likely to receive a counterfeit R4 than a counterfeit AK2i. Though there is a chance that many counterfeit versions of the listed flashcarts will work fine, there is too much variance to guarantee that any counterfeit is supported. As such, use this tool at your own risk.

## Installation/Usage
Instructions are documented at [@Plailect's](https://twitter.com/plailect?lang=en) [3ds.guide](https://3ds.guide) and are heavily recommended over any alternatives.

## Compiling
To compile this, you need:
- [devkitARM r47](https://3dbrew.org/wiki/Setting_up_Development_Environment)
- [firmtool](https://github.com/TuxSH/firmtool)
- [flashcart_core](https://github.com/ntrteam/flashcart_core) - This is bundled as a submodule in ntrboot_flasher. When cloning the repo, you **must** use the --recursive option to grab it. Example: `git clone --recursive https://github.com/ntrteam/ntrboot_flasher.git`
- [libelm3ds](https://github.com/Normmatt/libelm3ds) - This is bundled as a submodule in ntrboot_flasher. When cloning the repo, you **must** use the --recursive option to grab it.

## Planned support/Community involvement
While we initially planned to support more carts on release, we have not been able to expand beyond the current set of supported carts due to a lack of time/energy/manpower. If you have a cart you would like supported and have the know-how for debugging/reverse engineering a flashcart, please come find one of the devs in [#Cakey on freenode](https://kiwiirc.com/client/irc.freenode.net/?nick=ntrbootuser_?#Cakey). Ideally, this toolset is extensible towards any updatable Nintendo DS/DSi flashcart. See [flashcart_core](https://github.com/kitling/flashcart_core) for more flashcart details. **Do not submit a "feature request" on this repository asking "When will <my cart\> be supported?" The answer to this question is "We don't know". See [here](https://github.com/kitling/flashcart_core#requesting-support-for-a-new-card) for details on requesting your cart."**

## Licensing
This software is licensed under the terms of the GPLv3.
You can find a copy of the license in the LICENSE file.

## Credits
[@Normmatt](https://github.com/Normmatt) for initial implementation, bug squashing, expertise... etc.
[@SciresM](https://twitter.com/SciresM) for sighax/boot9strap and flashcart RE.
[@hedgeberg](https://twitter.com/hedgeberg) for testing and flashcart RE.
[@stuckpixel](https://twitter.com/pixel_stuck) for testing.
[@Myria](https://twitter.com/Myriachan) for testing.
[@Hikari](https://twitter.com/yuukishiroko) for testing.

Huge props to [@d3m3vilurr](https://twitter.com/d3m3vilurr) for figuring this out independently!
