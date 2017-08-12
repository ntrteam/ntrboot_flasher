# ntrboot_flasher
_A tool to flash that bootrom-hax goodness to your flashcart._

## Supported Cards
 - Acekard 2i HW-44
 - Acekard 2i HW-81
 - R4i Gold 3DS RTS

Note that in some rare circumstances, it may be possible for the flashing
process to **brick** a counterfeit flashcart and render it permanently
unusable. This is unlikely, but nevertheless only original flashcarts on the
list are supported. There is not a good way to test for this, unfortunately,
and its more likely to receive counterfeit R4s than AK2i's, though most have a 
high chance of working. There's just too much variance in these devices to 
gurantee any chance of certainty. As such, use at your own risk.

## Installation/Usage
Instructions are documented at 3ds.guide and are heavily recommended.

## Planned support/Community involvement
Initially we planned to have more cards supported on release, but have not been 
able to implement due to lack of time/energy. If you have a cart you would like
supported and have the knowhow for debugging/reverse engineering a flashcart,
please come find one of the devs in #Cakey on freenode or submit a feature 
request. Ideally, this toolset is extensible towards any updatable nintendo ds/
dsi flashcart. See [flashcart_core](https://github.com/kitling/flashcart_core) 
for more flashcart details. 

## Credits
@Normmatt for initial implementation, bug squashing, expertiese... etc.  
@SciresM for sighax/boot9strap and flashcart RE.
@hedgeberg for testing and flashcart RE.
stuckpixel for testing.  
Myria for testing.  

Huge props to @d3m3vilurr for figuring this out on their own!
