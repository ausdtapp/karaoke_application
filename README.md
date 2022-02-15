# karaoke_application
Repository for karaoke machine application.

This is a desktop application for windows built in C++ using the third party libraries glfw, ffmpeg, PortAudio, the MySQL xdev api.

Capabilities:

A backend MySQL database with a table of karaoke songs with columns for artist, song, and an MP4 file location. 
Decodes audio and video frames of the MP4s of available songs using ffmpeg.
Plays video in a window created with glfw (OpenGL) simlutaneously with audio using PortAudio.
Has a menu system that moves between a main menu, song selection, song results, and song streaming.
Visualized text entry for the user when searching for songs.

Program flow:

Initially a window is created asking the user to enter song search (but pressing the return key), after that the user can type a song name to query (which is finished by pressing 
the return key again), then the entry is queried in the MySQL table and the results are printed in the window to the user. After selecting a song to play the song's MP4 file 
location is used to decode the audio and video frames of the file, then the video frames are rendered and audio frames are played simultaneously for the karaoke experience. 
After the song finishes it will switch back to the song search menu and wait for another user submitted search.
