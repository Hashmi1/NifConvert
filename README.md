NifConverter
============

Copyright (c) 2012-2013, NIFUtilsSuite. All rights reserved SkyFox69
See https://github.com/Hashmi1/NifUtilsSuite/blob/master/docs/LICENSE.TXT for more licensing information.

This is my modification of NIFUtilsSuite-Lite to go with IndorilConverter:

- Its now a command-line application so that it can be used for automated batch-processing as needed by IndorilConverter.
- Collision-Addition and Nif-Conversion have been integrated so they both happen in a single go.
- I've added an option to make animated Skyrim Doors (non-loading) from Morrowind doors.

Usage:

NifConvert -in input_file_name -out output_file_name -[MODE] -txtr_prefix texture_prefix
		 
	-MODE can be either one of:
	    -stat
	    -door_anim
	    -door_load

All paths must be full-qualified paths (e.g. c:\\games\\skyrim\\data\\meshes\\example.nif )   ;
texture_prefix is the path to append to textures in the nif. e.g textures\\architecture\\  ;

