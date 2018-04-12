# cache-simulator
This project is part of the class CSE 140 Computer architecture at UC Merced. The project is to build a MIPS cache simulator using TIPS.

### Compiling

Install gtk-2.0
```
sudo apt-get install gtk2.0 

# The following may not be necessary, but in case the above doesn't get this by default:
sudo apt-get -f install
sudo apt-get install build-essential libgtk2.0-dev
```

If on Windows, install an X server like [Xming](http://www.straightrunning.com/XmingNotes/) and `export DISPLAY=:0`

Then just `make`, and `./tips` to run
