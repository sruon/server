------------------------------------
-- Rhapsodies of Vana'diel Job SQL Adjustments
-- This module reverts relevant SQL tables for jobs to their pre-RoV values
------------------------------------

------------------------------------
-- Monk
-- Source: https://forum.square-enix.com/ffxi/threads/52969
------------------------------------

-- Boost: Revert recast from 60 to 15 seconds
UPDATE abilities SET recastTime = 15 WHERE name = 'boost';

-- Focus: Revert recast from 2 to 5 minutes
UPDATE abilities SET recastTime = 300 WHERE name = 'focus';

-- Dodge: Revert recast from 2 to 5 minutes
UPDATE abilities SET recastTime = 300 WHERE name = 'dodge';

-- Chakra: Revert recast from 3 to 5 minutes
UPDATE abilities SET recastTime = 300 WHERE name = 'chakra';

-- Max HP Boost: Revert trait levels to 35/55/70
UPDATE traits SET level = 35 WHERE name = 'max hp boost' AND job = 2 AND rank = 2;
UPDATE traits SET level = 55 WHERE name = 'max hp boost' AND job = 2 AND rank = 3;
UPDATE traits SET level = 70 WHERE name = 'max hp boost' AND job = 2 AND rank = 4;

------------------------------------
-- Paladin
------------------------------------

-- Rampart: Revert recast from 3 to 5 minutes
-- Source: https://forum.square-enix.com/ffxi/threads/56444-February-12-2020-%28JST%29-Version-Update
UPDATE abilities SET recastTime = 300 WHERE name = 'rampart';

------------------------------------
-- Beastmaster
------------------------------------

-- Fight: Revert recast from 5 to 10 seconds
-- Source: https://forum.square-enix.com/ffxi/threads/50942
UPDATE abilities SET recastTime = 10 WHERE name = 'fight';

------------------------------------
-- Ranger
------------------------------------

-- Velocity Shot: Revert recast from 1 minute to 5 minutes
-- Source: https://forum.square-enix.com/ffxi/threads/55263-April.-3-2019-%28JST%29-Version-Update
UPDATE abilities SET recastTime = 300 WHERE name = 'velocity_shot';

------------------------------------
-- Dragoon
-- Source: https://forum.square-enix.com/ffxi/threads/54901-January.-10-2019-%28JST%29-Version-Update
------------------------------------

-- Jump / Spirit Jump: Revert to share a cooldown
UPDATE abilities SET recastId = 158 WHERE name = 'jump';
UPDATE abilities SET recastId = 158 WHERE name = 'spirit_jump';

-- High Jump / Soul Jump: Revert to share a cooldown
UPDATE abilities SET recastId = 159 WHERE name = 'high_jump';
UPDATE abilities SET recastId = 159 WHERE name = 'soul_jump';

------------------------------------
-- Puppetmaster
-- Source: https://forum.square-enix.com/ffxi/threads/50942
------------------------------------

-- Deploy: Revert recast from 5 to 10 seconds
UPDATE abilities SET recastTime = 10 WHERE name = 'deploy';

-- Retrieve: Revert recast from 5 to 10 seconds
UPDATE abilities SET recastTime = 10 WHERE name = 'retrieve';
