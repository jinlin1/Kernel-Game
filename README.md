This is a game similar to mastermind, but uses the kernel to simulate the game.

Run the makefile to compile the load.

Load the module into the kernel

```
sudo insmod mastermind.ko
```

To reomove the module from the kernel use

```
sudo rmmod mastermind.ko
```

Communicate with the module by writing to the devices associated with module

Start the game with

```
echo -n "start" > /dev/mm_ctl
```

Make a guess by using

```
echo -n "2000" > /dev/mm
```

All guesses are stored in a memory map. 
```
./memorymap
```

End the game with
```
echo -n "quit" > /dev/mm_ctl
```
