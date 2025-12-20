Deadlock Server Picker
======================

This program allows to block specific [Valve Relay Servers](https://developer.valvesoftware.com/wiki/Steam_Datagram_Relay)
in order to manipulate which servers Deadlock can connect to.

The program doesn't interfere with game files in any way. The working principle
is the same as in numerous similar programs for CS2: it blocks Valve servers
using your firewall, so the game thinks that those servers are down. The
difference is that CS2 and Deadlock use slightly different set of servers,
which was the reason I made this program.

## How to use

- Make sure your firewall is enabled.
- Launch the program, click "Sync server list".
- The program requires admin privileges in order to add rules to your firewall.
- Select servers you wish to block.
- That's it.

## For Linux users

> [!NOTE]
> Don't forget to `chmod +x DeadlockServerPicker`.

> [!WARNING]
> Changes are not persistent, they don't survive reboot.

Deadlock Server Picker needs privileges to alter nftables. There are two ways to run
the executable with sufficient privileges:

Simple way with `sudo`:
```
sudo ./DeadlockServerPicker
```

Correct way with `setcap`:
```
sudo setcap cap_net_admin+ep DeadlockServerPicker
./DeadlockServerPicker
```

Running `setcap` is required once, not before every run.

## Why wxWidgets?

For the 2000s nostalgia.
