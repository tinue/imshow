imShow 2.02, January 1993

imShow - An OS/2 2.0 application to display pictures in the formats:
  BMP  (OS/2 1.1, Windows 3 and OS/2 2.0 formats)
  GIF  (only plain, non-interlaced GIF87)
  im   (an image file format developed at the University of ZÅrich)

The comments of the source code are now in english, so I think I can
release it.


The "new" thing on this picture viewer is the support of the OS/2 2.0
palette manager. This means, you can see your pictures with their
actual colors!

It now has limited support on other devices than XGA that support the
palette manager (tested on Tseng ET4000). The palette manager on this
driver is that awfully buggy, that it is almost impossible to display
anything at all. Raja Thiagarajan found a way to display bitmaps
at least non-scaled (in 100% size). So that's what you can do now:
look at this pictures as long as you don't touch the "View" menu.
Of course on XGA and XGA-2 it still works as usual.


Requirements:
  OS/2 2.0 with service pack installed (to get the palette manager).
  A display adapter and driver which supports 256 colors and the
   palette manager (tested on the IBM XGA and XGA-2, limited function
   on Tseng ET4000).

Changes in 2.01:
  - Gif's are read in a bit faster (but still slowly).
  - Not all memory was freed when a new image was opened, resulting
    in an always increasing memory demand; This is fixed.
  - Gif's with less than 8 bit colors (e.g. 7 bit gif's) now don't
    allocate all 256 palette entries.
  - When "fit in window" was seleced, and the window had exactly
    the same dimensions as the bitmap, the image became corrupted.
  - Some code redundancies ar removed, resulting in better readable
    source code.

Changes in 2.02:
  - imShow now works also on Tseng Cards (as long as you don't
    touch the "View" menu.)
  - "Under the hoods" is quite a lot of rewrite done, so maybe
    there are now some new bugs...

Known bugs:
  - The shortcut "Ctrl-W" doesn't work (I have no idea why)
  - Sometimes OS/2 doesn't repaint the client area correctly,
    resulting in wrong colors on part of the image. This is a
    bug in PM.
  - Pictures can only be scaled on XGA and XGA-2 cards, not on
    any other card which supports the palette manager.
