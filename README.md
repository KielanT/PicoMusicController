# Pico Music Controller
![Pico Music Controller_bb](https://github.com/user-attachments/assets/9473e9f8-b7a4-4278-88f7-908a6b42bd45)


# Description
The Pico Music Controller is a project that turns a Raspberry Pi Pico (not W!) into a USB device for controlling Spotify. The Pico communicates with a desktop app to make calls to the Spotify Web API.

I created this project to address some annoyances I had while using Spotify. I always use Spotify while programming, and sometimes I want to pause/play, change the song, or adjust the volume. Usually, this requires opening the Spotify app. Although my keyboard has media buttons that reduce the need to open the app, the volume button affects the entire system sound rather than just Spotify. Additionally, if I have a video playing in the background, the buttons can control that instead of Spotify. This project solves those issues and helps boost productivity.

There are serveral branches in this project, I have explanation further down of each branch, but I have kept older versions of this project to help people to understand, what everything is doing.

### Intructions

# Spotify Web Api
During this project, I created a simple C++ wrapper for the Spotify Web API, which I decided to call CPPify. I've extracted the code and placed it into a separate repository for anyone who wants to use it without needing this project.

This project requires a text file named SpotifyDevCredentials.txt, which will store your Client ID and Client Secret. To obtain your Client ID and Secret, follow the instructions here: https://developer.spotify.com/documentation/web-api/concepts/apps

Make sure that the first line of the file contains the Client ID, and the second line contains the Client Secret. For security reasons, you must create this text file outside of the project folder to keep the Client Secret private and prevent it from being publicly accessible on GitHub.

On the first time of using this app you will directed to login to spotify, this will allow you to control the music playing on the Spotify Desktop app. This project uses Spotify Authorization Code flow, it is worth noting if you would like to expand on this product, you may need to change the Authorization type, which maybe more approiate for you project. More details can be found here: https://developer.spotify.com/documentation/web-api/concepts/authorization

If you would like to expand and make more calls to the Spotify Web API, you can find the calls in the same links above on the left side of the documentation under reference. 

# Project Ideas 
Since the majority of people viewing this project are likely hobbyists, I’ve created a list of potential projects that can be built based on my work. I would love to see what people create!

- PCB and Enclosure instead of a breadboard
- Use a Pico W instead, (remove the need for the desktop app)
- Create a UI in the window 
- Change the potentiometer to two buttons for the volume instead
- A bigger screen or scroll the text on the display

# Branches

### Console branch
The console branch doesn't use the Pico at all; it served as a testing ground for me to learn how to make calls to the Spotify Web API. I recommend checking it out to learn the basics of how these API calls are made.

### Simple branch
This branch is from before I implemented a window, so the project runs in a console. The advantage of this is that it's slightly easier to understand what's happening, especially if you're not familiar with multi-threading.

### Main branch
This is the most up-to-date version of the project and the one I recommend everyone use. It's also the version that the release package is based on.

# Problems
- There is a bug that sometimes occurs, when the Spotify desktop app becomes inactive (not being used) and then you press a button connected to the pico, it will sometimes start and pause.
- When the potentiometer (volume dial) is on some values, the value can start to fluctuate, I have prevented this by smoothing the values and adding a threshold value on the Pico side however, this still sometimes occurs, on some values, this can be fixed by putting the volume on a different value or by changing the threshold. The problem that happens is that it starts constantly updating the volume if its on a value that fluctuates
- When running release mode inside Visual Studio on closing app an unhandled exception is thrown


# Build Windows App binary

### Dependencies 



# Pico

### Pico circuit

### Pico Build


