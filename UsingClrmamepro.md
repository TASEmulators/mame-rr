**[clrmamepro](http://mamedev.emulab.it/clrmamepro/)** is a tool that checks your ROMs against a database.
It reports any missing or incorrect data and can correct some errors.

This is important because MAME and its variants refuse to run any software that isn't in its database, which changes with every version. Even with emulators that run any ROM, you should know which ROMs are good and bad and have them properly identified. Processing ROM sets with clrmamepro is much more efficient than manually managing the files.

**This guide only explains the core functions and basic usage of the program. See index.htm in its docs folder for more detail.**

Another guide can be found here:
http://www.mameworld.info/easyemu/clrmameguide/clrmame-menu.html

---



[Download the program.](http://mamedev.emulab.it/clrmamepro/download.htm) There are 32-bit and 64-bit flavors available, in installable (exe) and portable (zip) form. Install or extract it and run it.


---

# Create a new profile #

![http://mame-rr.googlecode.com/svn/wiki/profiler.png](http://mame-rr.googlecode.com/svn/wiki/profiler.png)

clrmamepro starts in Profiler mode, where you designate a database to use. The two ways to do this are to feed a prepared datfile to the profiler, or to to extract a dat directly from the emulator binary.

The second method saves you the step of getting a separate dat but can only be used with a few emulators. It works with MAME so we'll do that now. Click _Create_ and browse for the MAME binary. Put the version in the description and click _Create Profile_.

![http://mame-rr.googlecode.com/svn/wiki/profile-create.png](http://mame-rr.googlecode.com/svn/wiki/profile-create.png)

If you want to manage other sets you may want to put this profile in a subfolder just for MAME.

![http://mame-rr.googlecode.com/svn/wiki/profile-folder.png](http://mame-rr.googlecode.com/svn/wiki/profile-folder.png)

Now select the new profile from the tree on the left and the list on the right. Click _Load / Update_ and then _OK_ to the database update prompt.

![http://mame-rr.googlecode.com/svn/wiki/profiler-db-update.png](http://mame-rr.googlecode.com/svn/wiki/profiler-db-update.png)

This step will take some time and will stop a few times with warnings or questions. Pick _OK TO ALL_ and _Yes to All_ to clear them.

![http://mame-rr.googlecode.com/svn/wiki/profiler-datfile-problem2.png](http://mame-rr.googlecode.com/svn/wiki/profiler-datfile-problem2.png)

You still have to tell the program where your ROMs are, so click _Settings_ in the main window.

![http://mame-rr.googlecode.com/svn/wiki/main-settings.png](http://mame-rr.googlecode.com/svn/wiki/main-settings.png)


---

# Configure settings #

![http://mame-rr.googlecode.com/svn/wiki/settings.png](http://mame-rr.googlecode.com/svn/wiki/settings.png)

Pick _ROM-Paths_ from the dropbox, click _Add_, and browse to where your ROMs are. Add as many folders as necessary. You can also drag and drop instead of browsing.

clrmamepro will see anything in those paths that isn't in the dat as a bad ROM. If you keep subfolders in your ROM folders with unrelated files, pick _Exclude-Paths_ from the dropbox and add those subfolders. To ignore individual files, pick _Unneeded-Masks_ and add the filenames, with wildcards like `*.txt` if desired.

Click the _X_ to return to the main menu. Click _Scanner_.

![http://mame-rr.googlecode.com/svn/wiki/main-scanner.png](http://mame-rr.googlecode.com/svn/wiki/main-scanner.png)


---

# Scan #

![http://mame-rr.googlecode.com/svn/wiki/scanner.png](http://mame-rr.googlecode.com/svn/wiki/scanner.png)

_Sets_ are the zipfiles that contain individual _ROMs_. You'll want both of those checked. _CHDs_ are like ROMs but larger, and their files are placed in subfolders rather than zips. They come from media like hard disks and CDs and are compressed according to their own format. Many popular games require CHDs so you'll probably want to leave this checked. Samples are audio recordings to supplement older games with unemulated sound. If you don't have any you want scanned, uncheck _Samples_. If you do, you'll need to define paths for them back in _Settings_.

MAME dats organize games that share data, such as regional variants of the same game, into families. One of the members is arbitrarily selected as parent and the rest are clones. There are three ways to handle game families: _Non-Merged Sets_ means that all games, including clones, have all the data needed to run copied into their zipfiles. _Merged Sets_ means that there's only one zipfile for each family, with the clones moved in with the parents. In practice everyone uses _Split Sets_, which keeps parents and clones in separate zipfiles without duplicating the redundant parent data. (Sets may also be kept uncompressed in subfolders rather than zipfiles.)

To set the scanner to fix everything it can, click the small button under _Fix_ to fill all the boxes. To just check and report, click the button to clear all the boxes. If the scanner is told to fix _Unneeded_, then unrecognized files and bad ROMs will be moved to the backup folder, specified in Settings. If the other fix options are enabled, clrmamepro will correct any wrong ROM file and zipfile names and move ROMs to the correct zipfile if they are wrongly placed.

With _Ask before Fixing_ enabled, clrmamepro will stop to prompt you whenever it encounters something it can fix. Uncheck it to fix automatically.

Click _New Scan_ to begin. This process may take awhile.


---

# Review the results #

![http://mame-rr.googlecode.com/svn/wiki/scanner-results.png](http://mame-rr.googlecode.com/svn/wiki/scanner-results.png)

The Scan Results window shows info on a set-by-set basis. Only sets with problems are shown.

You can access a menu of things to do with this information by right-clicking in the results window. If you have a small subset of the collection, you may want to enable _View > Hide Fully-Missing Sets_. Changes to viewing options may not take effect until the next scan.

Click the _Set Information_ button to open another window with more in depth information about the status of all the sets.

The Statistics window pops up if _Add/Show Statistics_ was checked or if you click the _Statistics_ button. It reports the overall status of the profile, like how many ROMs are missing, and what problems were found and fixed.

![http://mame-rr.googlecode.com/svn/wiki/scanner-statistics.png](http://mame-rr.googlecode.com/svn/wiki/scanner-statistics.png)

Any unneeded files that were fixed were moved to the profile's backup folder, which is defined in _Settings_.

The initial scan creates a cachefile for the profile, which enables the _Scan_ button. Clicking this button skips the sets that had no problems and is faster than _New Scan_.


---

# Rebuilder #

![http://mame-rr.googlecode.com/svn/wiki/main-rebuilder.png](http://mame-rr.googlecode.com/svn/wiki/main-rebuilder.png)

So you obtained missing files to add to the profile. Use the Rebuilder to do the adding.

The Rebuilder examines each file you pass to it, checks it against the dat, and either incorporates it into the sets or ignores it if unrecognized.

![http://mame-rr.googlecode.com/svn/wiki/rebuilder.png](http://mame-rr.googlecode.com/svn/wiki/rebuilder.png)

You can type or browse the location of the files to be processed in _Source_, but it's easier to drag and drop onto the Rebuilder window. If you suspect that the Scanner discarded some good files, click the _UseBackupPath_ button to make the Rebuilder recheck them. By default the destination is the first ROM path defined in _Settings._

Merge options are the same as in the Scanner. The _Compress Files_ option can be disabled to yield subfolders instead of zipfiles. There's no need to use _Recompress Files_. Enabling _Remove Matched Sourcefiles_ moves the good files from source to destination, while disabling it makes copies instead.

![http://mame-rr.googlecode.com/svn/wiki/rebuilder-drop.png](http://mame-rr.googlecode.com/svn/wiki/rebuilder-drop.png)

After the rebuild, statistics are shown.

![http://mame-rr.googlecode.com/svn/wiki/rebuilder-statistics.png](http://mame-rr.googlecode.com/svn/wiki/rebuilder-statistics.png)

You may wish to return to the Scanner to rescan the profile now. Click the left radio button in the lower-right corner of the window for a shortcut to switch over.


---

# Keeping multiple arcade profiles #

Some ROMs may be different between different versions of MAME/FBA. If you need to run the same games on different versions, you may have to keep multiple versions of those games. Simply create a profile for each emulator and give them different ROM paths in their respective Settings.


---

# Other profiles #

The techniques here can be used to keep track of other dats besides MAME. For example, [no-intro](http://datomatic.no-intro.org/) and [redump.org](http://redump.org/) maintain software databases via dat files, and FBA can create a dat for itself from the menu with _Misc > Generate dat file > XML ClrMame Pro format > Generate dat_.

![http://mame-rr.googlecode.com/svn/wiki/profiler-add-dat.png](http://mame-rr.googlecode.com/svn/wiki/profiler-add-dat.png)

To add a dat, click _Add DatFile_ in the profiler, then browse to the dat you obtained. Pick a folder and click _OK_ to create the profile. An alternative way to add a dat is to place it directly where you want it in the datfiles subfolder of clrmamepro's directory, then click _Refresh List_ in the Profiler.

Click _NEW DATFILES_ in the pane on the left and then the new profile on the right. Click Load / Update to commence the same procedure as above.


---

# Updating profiles #

Stay up to date with new versions of your profile.

If the dat for the profile was extracted directly from an emulator, simply overwrite the old binary with the new. Select the old profile in the profiler and click _Load / Update_. clrmamepro will detect the change and prepare a new datfile.

If the dat was downloaded, follow the same procedure for adding it as a new dat. When you attempt to load the new profile, clrmamepro will detect that it is an update and proceed accordingly.

After updating a profile that has already been scanned, the _Diff Scan_ button becomes available in the Scanner. This scans only sets that changed in some way since the last version, so it's faster than a _New Scan_