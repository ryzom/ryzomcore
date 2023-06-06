-- In this file we define functions that serves for inventory window
------------------------------------------------------------------------------------------------------------
-- create the game namespace without reseting if already created in an other file.
if (game==nil) then
	game= {};
end

function game:initEquipPreview()
    setChar3dDBfromServerDB('UI:TEMP:PREVIEWCHAR3D')

    local scene = getUI("ui:interface:inventory:content:equip:midsection:content:visu3d:char3d")
    local headz = scene:getElement("character#0").headz
    local pz = scene:getElement("character#0").posz
    local height = headz - pz
    local camera = scene:getElement("camera#0")
    camera.tgtz = pz + height*0.55
    camera.posz = camera.tgtz + 0.5
    camera.posy = 26 - 1.6*(height)
    scene.distlimitmin = 25 - camera.posy
    scene.distlimitmax = 27 - camera.posy

    scene = getUI("ui:interface:inv_equip:content:equip:midsection:content:visu3d:char3d")
    headz = scene:getElement("character#0").headz
    pz = scene:getElement("character#0").posz
    height = headz - pz
    camera = scene:getElement("camera#0")
    camera.tgtz = pz + height*0.55
    camera.posz = camera.tgtz + 0.5
    camera.posy = 26 - 1.6*(height)
    scene.distlimitmin = 25 - camera.posy
    scene.distlimitmax = 27 - camera.posy
end

function game:updateEquipOnResize(base, force)
    local path = "ui:interface:"..base
    local win = getUI(path)
    local equipPath = path..":content:equip"
    local equip = getUI(equipPath)
    if equip ~= nil and (equip.active ~= false or force) then
        local w = win.w
        local h = win.h
        local hotbar = getUI(equipPath..":hotbar_c")
        local hotbarTitle = getUI(equipPath..":hotbarTitle")
        local midsection = getUI(equipPath..":midsection")
        local jewelry = getUI(equipPath..":jewelry")
        local armors = getUI(equipPath..":armors")
        local handl = getUI(equipPath..":handl")
        local handr = getUI(equipPath..":handr")
        if h < 345 then
            hotbar.active = false
            hotbarTitle.active = false
        else 
            hotbar.active = true
            hotbarTitle.active = true
        end
        if w < 450 then
            midsection.active = false
            jewelry.x = -10
            armors.x = 10
            jewelry.y = 0
            armors.y = 0
        else 
            midsection.active = true
            jewelry.x = -20
            armors.x = 20
            jewelry.y = -5
            armors.y = -5
        end
        if w > 390 and h > 344 then
            handr.active = true
            handl.active = true
        else
            handr.active = false
            handl.active = false
        end
    end
end
-- VERSION --
RYZOM_INVENTORY_VERSION = 10469