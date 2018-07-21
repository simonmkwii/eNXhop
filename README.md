# FreeShopNX
Nintendo Switch CDN title installer, based on Rei's BogInstaller (evolution of eNXhop)

Requirements:
* Sig Patches (SX OS)
* `sdmc:/switch/FreeShopNX/FreeShopNX.nro`
* `sdmc:/switch/FreeShopNX/Ticket.tik`
* `sdmc:/switch/FreeShopNX/Certificate.cert`
* `sdmc:/switch/FreeShopNX/FreeShopNX.txt`

`Ticket.tik` and `Certificate.cert` are not supplied in this repo, you will need to track them down and obtain them yourself. They are the same ones used in CDNSP. 

`FreeShopNX.txt` contains the Right ID, Title Key, and Title Name of any titles you wish to install, comma separated. It uses the same format as a CSV file, simply renamed to TXT. Example below:

```
01001de0050120000000000000000003,XXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXX,Quest of Dungeons
01002b30028f60000000000000000004,XXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXX,Celeste
01000000000100000000000000000003,XXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXX,Super Mario Odyssey
01007ef00011e0000000000000000000,XXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXX,The Legend of Zelda: Breath of the Wild

```

Right IDs are the 16 character Title ID followed by the Master Key revision the game uses padded with leading zeros to 16 characters. Right IDs can be obtained from the games CNMT or by using hactool on the game's NCA.

No TItle Keys or completed `FreeShopNX.txt` file will be provided from this repo. 

Credits to Adubbz for Tik and Cert installation with TinFoil, XorTool for the eNXhop base and UI, Reisyukaku for on-system title installation via BogInstaller, and SimonMKWii for the icon.

Note: You'll probably get banned. 