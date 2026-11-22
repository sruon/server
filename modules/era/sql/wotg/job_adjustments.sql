------------------------------------
-- Wings of the Goddess Job SQL Adjustments
-- This module reverts relevant SQL tables for jobs to their pre-WotG values
------------------------------------
-- Source : https://www.bg-wiki.com/ffxi/Version_Update_(04/08/2009)
------------------------------------

------------------------------------
-- White Mage
------------------------------------

-- Martyr: Revert range from 20 to 6 yalms
UPDATE `abilities` SET `range` = 6.0 WHERE `name` = 'martyr';

------------------------------------
-- Beastmaster
------------------------------------

-- Reward: Revert recast from 1 1/2 min to 3 minutes
-- Source: https://www.bg-wiki.com/ffxi/Version_Update_(03/11/2008)
UPDATE `abilities` SET `recastTime` = 180 WHERE `name` = 'reward';

-- Sic: Allow use with jug pets in addition to charmed mobs
-- Source: https://www.bg-wiki.com/ffxi/Version_Update_(11/09/2009)
UPDATE `abilities` SET `addType` = 192 WHERE `name` = 'sic';

-- Heel: Revert recast from 5 seconds to 10 seconds
-- Source: http://www.playonline.com/pcd/update/ff11us/20071120wwnX41/detail.html
UPDATE `abilities` SET `recastTime` = 10 WHERE `name` = 'heel';

-- Stay: Revert recast from 5 seconds to 10 seconds
UPDATE `abilities` SET `recastTime` = 10 WHERE `name` = 'stay';

-- Leave: Revert recast from 5 seconds to 10 seconds
UPDATE `abilities` SET `recastTime` = 10 WHERE `name` = 'leave';

-- Heel/Stay/Leave: Remove shared recast
UPDATE `abilities` set `recastId` = 152 WHERE `name` = 'heel';
UPDATE `abilities` set `recastId` = 153 WHERE `name` = 'leave';
UPDATE `abilities` set `recastId` = 154 WHERE `name` = 'stay';
