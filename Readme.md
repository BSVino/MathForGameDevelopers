Math for Game Developers Sample Code
====================================

This is the source code for the sample code that I use in my [Math for Game Developers YouTube series](http://www.youtube.com/playlist?list=PLW3Zl3wyJwWOpdhYedlD-yCB7WQoHf-My). If you want to follow along with the videos, you can download by clicking the 'ZIP' button in the header near the top of this page.

Overview
--------

You'll find these directories:

**game** - This is where the main.cpp is, which is the primary bit of code that we'll be modifying.

**math** - This is where all of the math stuff that we'll be implementing lives.

**renderer** - Shaders and rendering are carried out here. All OpenGL code is contained in these files. To render something, first create a rendering context:

	CRenderingContext c(Application()->GetRenderer(), true):

If there's already a rendering context open then use that one, don't create two contexts in the same method.

Shaders are loaded from text files in the ```shaders``` directory in the SMAK distribution. The shader library will read every .txt file in this directory and compile it as a new shader. The .txt file specifies which file should be used as the vertex and fragment shaders, so files can be reused between shaders. It also specifies parameters which can be specified in the material .mat files. The files ```header.si``` and ```functions.si``` are imported for you at the beginning of every shader. Some attributes and uniforms are hard-coded. For vertex shaders:

* ```uniform mat4x4 mProjection```
* ```uniform mat4x4 mView```
* ```uniform mat4x4 mGlobal```

And for fragment shaders:

* ```out vec4 vecOutputColor```

```vecOutputColor``` is the value to which the output value for a fragment shader should be assigned. The rest are up to you.

**datamanager** - Sometimes data needs to be serialized to and from the hard drive. data.h contains a CData class that is a generic data container, and dataserializer.h can serialize any CData to and from a stream. Data is stored in a simple format that looks like this:

	key: value
	{
		// Sub-datas
		key: value
		key2: value

		// Values are optional
		key3
		{
			subdata
		}
	}

This format is used for things like shader .txt files (see below) and material .mat files.

Compiling
---------

This project has no dependencies and should compile out-of-the-box. If something doesn't work for you please let me know.

1. Download and install Visual Studio 2010.
2. Download the MFGD project files and unpack them somewhere to your hard drive.
3. Double click the MFGD.sln file. Visual Studio should appear.
4. Press F7 to compile the game.
5. Press F5 to run the game.

Want to send your game to a sexy friend? You'll need to do some extra steps.

1. Go to the dropdown in Visual Studio that says "Debug" and set it to "Release" instead. It should be near the top of the screen.
2. Press F7 to compile the game in "Release" mode.
3. Find the file MFGD.exe (it's an "Application" file type) and copy it over to the "content" folder.
4. Send this content folder to your sexy friend.

(The steps are the same for non-sexy friends.)

Right now only Windows compiling is supported, but I like patches!

Tinker
------

This code is built on top of Tinker, Lunar Workshop's internal game engine. I ripped out a bunch of stuff so that it's much more simpler. As MFGD progresses and new things are learned, I'll add those things back into the engine.

If you want the actual thing you can find it in the [SMAK source code](https://github.com/BSVino/SMAK), it's much more fully featured. It has materials and a GUI and lots of other neat stuff.

Licenses
--------

Tinker is licensed under a permissive MIT-style license, meaning you may use it to build your own projects with no requirement to distribute the source code, but you must give credit to the original author somewhere in your program, like on the "About" or "Credits" page.

The full text of the license is included in the file ```LICENSE.Tinker``` and at the top of every source code file to which it applies.
