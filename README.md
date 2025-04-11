Basic mod menu for Splatoon 2. This doesn't bypass anticheat or do anything special, basically a runtime pchtxt.

Thanks Shadów and Haxxie for exlaunch and imgui-nvn. Uses [oead](https://github.com/shadowninja108/slide) implementation from Shadów.

Uses patches based on SHUT 5.5.0 public patches

**Building:**
1. Install DevKitPro as well as all Switch dependencies (see [Atmosphère docs](https://github.com/Atmosphere-NX/Atmosphere/blob/master/docs/building.md)) into your favorite Linux distro (or WSL)
2. Download this repo into a folder named `imgui-nvn`
 ```bash
#Example, use whatever path you want as long as the folder is imgui-nvn
mkdir -p ~/Documents/Projects/splat2-test-mod-menu/imgui-nvn/
git clone https://github.com/liz403/splat2-test-mod-menu.git /home/liz/Documents/Projects/splat2-test-mod-menu/imgui-nvn/
   ```
3. Change whatever you want in the configs or code
4. Run `./exlaunch.sh clean && ./exlaunch.sh build`
