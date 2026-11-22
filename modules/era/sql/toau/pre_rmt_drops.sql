------------------------------------
-- Pre RMT Drops
-- This module reverts RMT changes from a ToAU era Patch in 2007
------------------------------------
-- Source : http://www.playonline.com/pcd/update/ff11us/20070308c2bbd1/detail.html
------------------------------------

DROP PROCEDURE IF EXISTS replace_drop;
DELIMITER $$
CREATE PROCEDURE replace_drop(
    IN zoneName TINYTEXT CHARACTER SET utf8mb4 COLLATE utf8mb4_general_ci,
    IN mobName TINYTEXT CHARACTER SET utf8mb4 COLLATE utf8mb4_general_ci,
    IN oldDropId SMALLINT UNSIGNED,
    IN newDropId SMALLINT UNSIGNED
)
BEGIN
    SET @zoneId = (SELECT zoneid FROM zone_settings WHERE `name` = zoneName);
    SET @dropListId = (SELECT dropid from mob_groups WHERE zoneid = @zoneId AND name = mobName);
    UPDATE mob_droplist SET itemId = newDropId WHERE dropId = @dropListId AND itemId = oldDropId;
END $$
DELIMITER ;

-- replace_drop('zone name', 'mob name', old item id, new item id);
-- zone name: as found in zone_settings.sql
-- mob name: as found in mob_groups.sql
-- old/new item id: as found in data/items.yaml

CALL replace_drop('FeiYin', 'Western_Shadow', 18752, 17472); -- retaliators -> cross-counters
CALL replace_drop('FeiYin', 'Eastern_Shadow', 18714, 17187); -- valis_bow -> eurytos_bow
CALL replace_drop('South_Gustaberg', 'Leaping_Lizzy', 15351, 13014); -- bounding_boots -> leaping_boots
CALL replace_drop('Castle_Oztroja', 'Mee_Deggi_the_Punisher', 14986, 13952); -- ochimusha_kote -> ochiudos_kote
CALL replace_drop('Castle_Oztroja', 'Quu_Domi_the_Gallant', 15737, 13054); -- sarutobi_kyahan -> fuma_kyahan
CALL replace_drop('Labyrinth_of_Onzozo', 'Lord_of_Onzozo', 18852, 17440); -- octave_club -> kraken_club
CALL replace_drop('Valkurm_Dunes', 'Valkurm_Emperor', 15224, 12486); -- empress_hairpin -> emperor_hairpin
CALL replace_drop('Maze_of_Shakhrami', 'Argus', 15515, 13056); -- peacock_amulet -> peacock_charm
CALL replace_drop('Jugner_Forest', 'King_Arthro', 15899, 13189); -- velocious_belt -> speed_belt
CALL replace_drop('Sauromugue_Champaign', 'Roc', 18587, 17108); -- dryad_staff -> healing_staff
CALL replace_drop('Rolanberry_Fields', 'Simurgh', 15736, 14080); -- trotter_boots -> strider_boots
CALL replace_drop('Ordelles_Caves', 'Stroper_Chyme', 15551, 13514); -- shikaree_ring -> archers_ring

-- Astral Ring (Coffer chests in Castle of Oztroja) handled in modules/era/lua/rmt_drops.lua

-- Define rate variables
SET @COMMON   = 150;  -- 15%
SET @UNCOMMON = 100;  -- 10%
SET @RARE     = 50;   -- 5%
SET @VRARE    = 10;   -- 1%

INSERT INTO `mob_droplist` VALUES (2588,0,0,1000,1313,@RARE); -- Sea Serpent Grotto - Voll the Sharkfinned - Siren's Hair (5%)
INSERT INTO `mob_droplist` VALUES (2813,0,0,1000,1313,@RARE); -- Sea Serpent Grotto - Zuug the Shoreleaper - Siren's Hair (5%)
INSERT INTO `mob_droplist` VALUES (1973,0,0,1000,1313,@RARE); -- Sea Serpent Grotto - Pahh the Gullcaller - Siren's Hair (5%)
INSERT INTO `mob_droplist` VALUES (2673,0,0,1000,1313,@RARE); -- Sea Serpent Grotto - Worr the Clawfisted - Siren's Hair (5%)
INSERT INTO `mob_droplist` VALUES (1825,0,0,1000,1313,@RARE); -- Sea Serpent Grotto - Novv the Whitehearted - Siren's Hair (5%)
INSERT INTO `mob_droplist` VALUES (128,0,0,1000,1312,@RARE);  -- Cape Terrigan - Devil Manta (Fished) - Angel Skin - (5%)
INSERT INTO `mob_droplist` VALUES (149,0,0,1000,836,@VRARE);  -- The Boyahda Tree - Aquarius - Damascene Cloth (1%)
