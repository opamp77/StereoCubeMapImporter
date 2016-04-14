# StereoCubeMapImporter


A simple stereo cubemap importer for displaying stereo cubemaps such as the ones availible [here](https://render.otoy.com/forum/viewforum.php?f=97)
within UE4.<br>
The Plugin take a pano stereo cubemap image splits it and generates two standard UE4 cubemaps.

## Installation and Use


If you are not compiling UE4's engine from source you should download one of the latest precompilied windows binaries:

* [Engine Version 4.9.2](http://www.mediafire.com/download/nziiz1ncf07wadg/StereoCubeMapImporter-ue4.9.2.rar)
* [Engine Version 4.10.2](http://www.mediafire.com/download/0ukvjd44zgd46s6/StereoCubeMapImporter-ue4.10.2.rar)
* [Engine Version 4.11.1](http://www.mediafire.com/download/do8auzx11a7bxbc/StereoCubeMapImporter-ue4.11.1.rar)

The contents of the plugin should be copied to your UE4 installation location.
So that the StereoCubemapImporter folder resides in the engines plugin folder.
i.e C:\UE4\Engine\Plugins\StereoCubemapImporter.


To import a stereo cubemap simple click the new button in the editor.
![](http://i.imgur.com/i5P4DDn.png)


The Plugin will generate a standard ue4 cubemap for each each eye.
To uses the cube maps please check out StereoCubeMap_MAT and StereoCubeMap_PP_MAT within the plugin's content folder.
![](http://i.imgur.com/vNSmPXR.png)
