# DREAM3D Release Notes #

## Important Changes in Version 4.2 ##

### TSL Reference Frame Changes ###

Versions prior to 4.2 did not correctly set the sample and crystal reference frame transformations when importing data into an .h5ebsd file. This bug has been corrected but due to changes within the .h5ebsd file the user is *strongly* encouraged to reimport any .ang data files an rerun any pipelines that depend on that data. More information can be found in the "Import Orientation Data" documentation.

### Changes to Internal Quaternion Structure ###
For historical compatibility in versions prior to 4.2 the internal representation of a quaternion was a 5 component array for each quaternion. This compatibility is no longer needed and the internal structure has been changed to a standard (X, Y, Z, W) style quaternion. This will create issues when loading .dream3d files that were created with versions prior to 4.2 into DREAM3D versions 4.2 and beyond. In order to alleviate migration issues a new filter in the "Update Cell Quaternions" located in the "Generic" section of the filter library will convert existing 5 component quaternions to the newer 4 component layout. If the user would like to simply update their .dream3d file the pipeline to accomplish this would be:

+ Read DREAM3D File (Check EVERY array to read)
+ Update Cell Quaternions
+ Write DREAM3D File (Check EVERY array to write)

**Note: This pipeline will OVERWRITE the existing DREAM3D File if both the input and output files are the same file.**

## Issue Tracker ##
 The official DREAM3D issue tracker is located at https://github.com/DREAM3D/DREAM3D/issues

## Version 4.2 Bugs Fixed & Features Added ##


### Version 4.2.4905 ###
+ Fixed Issue no. 147: Adding a favorite may result in the favorite being written to the wrong directory
+ Added several filters from the 'develop' branch to determine microtexture regions in Titanium alloys
+ Fixing preflight error in the _RenameFieldArray_ filter
+ Optimizing the EBSD Segment Grains filter for speed based on the random number generator
+ Adding additional methods to the OrientationMath and QuaternionMath c++ classes
+ Fixing issue with writing the parameters of some filters into a pipeline file
+ Fixed Issue no. 148: Fixing issue where an .ang file was too short with respect to the headers. We now return a warning to the user stating there was not enough data in the file
+ Fixing bug when adding a Favorite Folder the user type was not set correctly and subsequent adding of a favorite to the folder would not work correctly.


### Version 4.2.4815 ###
+ Added ability to nest Favorite Pipelines in folders
+ Fixed bugs when reading/write filter parameters for some Filters
+ Completely rewrote the GBCD codes to more closely mimic a reference implementation from CMU
+ Added the ability to generate several types of GBCd output files for visualization or use in an external GBCD code
+ Added version number to each Pipeline File
+ Fixed issue no. *143*: Fixing issue where reading in a stats file would not properly parse the ODF data
+ Fixed bug where doing certain types of rotation calculations would cause a crash or malformed output data
+ Adding a parameter to the Visualize GBCD filter to allow the user to set the Crystal symmetry
+ Fixed issue no. *138*: Convert data now works correctly on boolean Arrays
+ Many more low level developer centric bugs and APIs


### Version 4.2.4695 ###
+ Fixed issue no. *132*: Fixing issue where .ang files would error out on reading due to the file not being long enough based on the header information. The error has been changed to a warning for the user.
+ Fixed issue no. *134* Adding missing checks for FaceLabels array (GrainIds or Feature Ids) and presenting errors if that array is not found.
+ Fixed issue no. *133*: Adding option to group STL files by Phase by encoding the Phase index into the file name and the header of the STL file.
+ Fixed issue no. *135*: Ph reader only would read .dx file extensions.
+ Misc Fixes/Enhancements:
  + Fixing issue where the git commit hash always had a ‘g’ as the first character
  + Updating the Compile documents with the latest 3rd party library requirements
  + Adding an additional piece of meta data to write the exact version of DREAM3D as an attribute to the .dream3d file
  + Renaming the AbaqusFileWriter class to be more consistent with other naming schemes. Added some comments to the output Abaqus file regarding the type of elements being used.

## Version 4.2.4685 ###
+ Updating issue no. *114*: Adding Experimental "Write Pole Figure" filter which is only allowable for some Laue groups
+ Fixed issue no. *128*: Fixing Bugs in the Alignment filters due to mis-use of C++ inheritance rules.
+ Fixed issue no. *110*: Adding an "Update Favorite" to the DREAM3D contextual menu. This will update the selected favorite with the current pipeline.
+ Fixed issue no. *109*: Backporting all the Crystal Orientation Operations classes from the development branch to allow more functionality in certain filters
+ Fixed issue no. *129*: Fixing issue with "Import Orientation Data" filter when large EBSD scans were imported the Reference Frame Dialog would resize larger than the screen making use of the dialog impossible. This has been fixed with a QGraphicsView layout and basic zooming capabilities.
+ Fixed issue with the EBSD .ang reader to report an error if the "Phase" header entry is never found which can happen on some TSL "combo" scans.
+ Fixed issue no. *130*: Estimated Time remaining for Synthetic generation was not correct.
+ Fixed issue no. *125*: ReadH5Ebsd crashes if Z Spacing is 0 (Zero)
+ Updating the Euler to Rotation_Matrix and Rotation_Matrix to Euler functions with more robust implementations that will not cause a NAN value.
+ Updating the Misorientation Coloring Filter based off the C. Schuh/S. Patala paper. This only works for some Laue groups currently. See the documentation for the filter for those Laue groups that the filter will work for.
+ Setting automatic update check to be manual by default.


### Version 4.2.4635 ###
+ Adding crystal symmetry operations for all crystal symmetries. This allows the IPF Coloring to work for all symmetries
+ Adding the Misorientation Coloring Filter based the Schuh & Patala algorithm and implemented by Will Lenthe at UCSB
+ Low level bug fixes effecting various IO filters

### Version 4.2.4629 ###
+ Fixing an issue with sampling the ODF properly when matching crystallography
+ Adding checks for resolution values of zero at which point the value is changed to 1.0.
+ Adding another entry for the Small IN100 .ang files
+ Fixing crashing errors for MinSizePerPhase due to inappropriate use of subclass functions
+ Fixing Documentation errors
+ Fixing no. *124*: Crash on Visual Studio using %zu as a format specifier in printf statements on Visual Studio. Effected the INLWriter
+ Fixing bad dimension output for the INLWriter
+ Updating the tutorial for reconstruction of the Small IN100 data set
+ Adding an IN100 prebuilt pipeline that reconstructs the entire Small IN100 data set in a single pipeline
+ Updating the synthetic tutorial to match the newer version of DREAM3D
+ Setting the origin values to 0 as a default for Initialize Synthetic Volume
+ Fixing error message reporting on generate ensemble statistics filter

### Version 4.2.97 ###
+ Fixed bug where the "Generate Ensemble Statistics" could not execute due to an error in the internal naming of variables.
+ Updated the documentation for the "Find Average Orientations" filter
+ Clarifying an error message that is produced when a filter requests an array from the Data Container but the array stored in the Data container does not match type and/or size and/or number of components.

### Version 4.2.90 ###

+ fixed bug in Quaternion Math where floats were being cast to integers for the absolute value calculation
+ Fixed bug in Find Surface Voxel Fraction where the type for the surfaceVoxel was not cast correctly in the data check method

### Version 4.2.88 ###
+ Adding filters for volume fraction and surface area to volume ratio
+ Fix bug in ScalarSegmentGrains where boolean arrays are not segmented correctly
+ On the raw binary reader allow user to optionally over ride the origin and spacing values with the entries in the filter.
+ Visualize GBCD, empty GMT file causes crash.
+ Added warning to GenerateEnsemble Statistics when the correct distribution types are not selected.
+ Fixed bug in MXAFileInfo where passing in an empty string to the "filename()" method would cause infinite recursion.
+ Incorrect Documentation for Find Twin Boundary Info
+ StatsGenerator: Fixed array access out of bounds crash because we were not checking the bounds of the ColorNames array before accessing.
+ StatsGenerator: Fix Tab sequence on Primary Phase Grain Size Distribution Tab
+ Update the documentation for the disorientation coloring
+ Add help for the Euler Reference Frame Dialog in the "Import Orientation Data" Filter.
    - Have it pop open the Web browser to the proper file.
    - Write up an .md file for the "Reference Frame Dialog" that explains the transformations that each radio button performs.
    - Take into account the Euler Reference Frame Rotations (Current does not matter because all rotations are about the Z Axis)
+ Add filter to calculate info for twin boundaries for UCSB guys
+ Read in the Field and Ensemble Arrays of the SurfaceMesh data container including updating the GUI for this.
+ Fix GBCD Generation or make private on next release
+ Make Euler Color Filter Private as the implementation is not correct.
+ Import OrientationData GUI, ReferenceFrameDialog assumes cubic crystal when calculating IPF Colors.
+ Create filter to dump color images based on an RGB Array (currently only the Generate IPF Colors creates these types of arrays)
+ Create Filter to change data types, ie, convert 16 bit ints to 32 bit ints.
+ Raw Binary Reader does not error out if we read off the end of the file
+ Add a filter to apply operations to the surface mesh such as moving the surface mesh in space coordinates (Verify what was written already)
+ Add Prebuilt Pipeline to generate 3 IPF Colors for 3 Orientations.
+ Disable the **Go** button if there are preflight errors on the DREAM3D Interface
+ Design protocol to check for updated Version of DREAM3D.
+ Error messages returned from Pipeline are hyperlinked to the filter docs.
+ Update the documentation for the Neighbor Cleanup Filters.
+ When removing a Favorite, present a standard dialog to make sure this is what they wanted to do.
+ Show Favorites or Prebuilt Pipelines in the OS X Finder or Windows Explorer
+ Append a favorite to a pipeline that is already built up in the PipelineView widget
+ incorrect Documentation for Neighbor CI Correlation
+ incorrect Documentation for Neighbor Orientation Correlation
+ FilterParameter: EditibleChoice Widget NOT firing updates to force the Preflight to run
+ Add the "Show User Manual" to the "Help" menu.
+ Create Helper Class in QtSupport that generates the proper URL to open an html file in the users browser since this code is reused all over the place.
+ Add a cleanup filter that considers the orientation of neighboring voxels when assigning bad voxels
+ Add TSL Neighbor CI correlation clean up filter
+ Add TSL Neighbor Orientation correlation clean up filter
+ Right-click on filter in the **Filter List** and display the help for that filter
+ Right click on FilterWidget title area and have context menu (Show Help, Remove, etc)
+ HexToSquare Convertor changed to put the "Sqr\_" as a prefix on the new files instead of a suffix which allows the files to be more easily recognized by the "ImportOrientationData" filter.
+ Fix EBSDLib to understand more Crystal Symmetries in TSL .ang files.
+ Synthetic Microstructure generation tutorial needs to be updated with the additional filters that are required for Match Crystallography to work properly.
+ Add a 'Rename Favorite' menu
+ Allow user to right-click on a favorite and "delete, rename, ..." the favorite
+ Make reading of the "Categories" a non fatal error from the H5Ebsd file as TSL Combo Scans do not include this header.
+ AutoScroll PipelineView when dragging a filter widget
+ Add filter to calculate triangle areas
+ Add filter to calculate minimum dihedral angle of triangles
+ ComparisonSelectionWidget ONLY works on Cell Arrays - Now works on Cell, Field, Ensemble, Point, face and Edge
+ Add info in the documentation files that state where a required array could be created from
+ Added a widget to draw attention to the 'Set Reference Frame' button which is yellow until they click the button.
+ Fixed slow calculation in StatsGenerator when initially calculating default statistics
+ Allow user to set the bin step size in generate ensemble statistics filter
+ Fix match crystallography to deal with matrix phases, etc.
+ Remove all "under-the-hood" calls to other filters
+ Add filter to set the Origin, Resolution of the voxel data container( Basically change the meta data with out needing to change any of the data)
+ write-up contributors section for website
+ Drag-and-Drop a pipeline file onto the DREAM3D UI and have it load that file
+ How to Incorporate documentation for plugins into the main help system
+ Break out all stats filters to just calculate stats
+ Update file type stats generator can read (.dream3d)
+ Users can not open .h5stats in StatsGenerator
+ Add ability to open a previously saved stats file back into the stats generator
+ Add filter to calculate only misorientations
+ Modify FindMDF filter to only calculate the MDF, not the actual misorientations
+ Add filter to calculate ensemble stats (fit distributions to arrays)
+ Fix calculation+storage of real stats for reading into synthetic builder
+ Read DREAM3D Filter: Not saving which SurfaceMesh arrays were checked to the file
+ Missing Documentation File for IOFilters/ImportImageStack
