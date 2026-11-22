------------------------------------
-- Abyssea Job SQL Adjustments
-- This module reverts relevant SQL tables for jobs to their pre-Abyssea values
------------------------------------
-- Unless otherwise noted, all changes here are sourced from: https://www.bg-wiki.com/ffxi/Version_Update_(03/26/2012)
------------------------------------

------------------------------------
-- Beastmaster
------------------------------------

-- Pet Food Biscuits: Remove level requirements
-- Source: https://www.bg-wiki.com/ffxi/Version_Update_(09/08/2010)
UPDATE item_equipment SET level = 0 WHERE name = 'pet_food_alpha';
UPDATE item_equipment SET level = 0 WHERE name = 'pet_food_beta';
UPDATE item_equipment SET level = 0 WHERE name = 'pet_fd._gamma';
UPDATE item_equipment SET level = 0 WHERE name = 'pet_food_delta';
UPDATE item_equipment SET level = 0 WHERE name = 'pet_fd._epsilon';
UPDATE item_equipment SET level = 0 WHERE name = 'pet_food_zeta';

------------------------------------
-- Bard
------------------------------------

-- Mazurka: Revert to town/field only
-- Source: https://www.bg-wiki.com/ffxi/Version_Update_(09/08/2010)
SET @TYPE_CITY     = 1;
SET @TYPE_OUTDOORS = 2;
SET @MISC_MAZURKA  = 8;

-- Remove mazurka from all zones
UPDATE zone_settings
SET misc = misc & ~@MISC_MAZURKA;

-- Reapply to cities and outdoor zones
UPDATE zone_settings
SET misc = misc | @MISC_MAZURKA
WHERE (zonetype & (@TYPE_CITY | @TYPE_OUTDOORS)) <> 0;
