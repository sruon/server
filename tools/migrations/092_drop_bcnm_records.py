import re

import mariadb

BATTLEFIELD_EFFECT = 254
VAR_PATTERN = re.compile(r"^Battlefield\[(\d+)\](.*)$")


def migration_name():
    return "Renumber battlefields to zone and index, keep their records and drop bcnm_records"


def check_preconditions(cur):
    return


def needs_to_run(cur):
    cur.execute("SHOW TABLES LIKE 'bcnm_records'")
    if cur.fetchone():
        return True
    return False


def migrate(cur, db):
    try:
        cur.execute("SELECT bcnmId, zoneId FROM bcnm_records")
        new_ids = {old_id: (zone_id << 5) | (old_id & 31) for old_id, zone_id in cur.fetchall()}

        cur.execute(
            """
            CREATE TABLE IF NOT EXISTS `battlefield_records` (
              `bcnmId` smallint(5) unsigned NOT NULL,
              `fastestName` varchar(15) NOT NULL,
              `fastestPartySize` tinyint(2) unsigned NOT NULL,
              `fastestTime` int(10) unsigned NOT NULL,
              PRIMARY KEY (`bcnmId`)
            ) ENGINE=InnoDB DEFAULT CHARSET=utf8mb4 COLLATE=utf8mb4_general_ci;
            """
        )
        cur.execute(
            """
            INSERT IGNORE INTO `battlefield_records` (bcnmId, fastestName, fastestPartySize, fastestTime)
            SELECT (zoneId << 5) | (bcnmId & 31), fastestName, fastestPartySize, fastestTime FROM `bcnm_records` WHERE BINARY fastestName <> 'nobody';
            """
        )

        cur.execute("SELECT charid, power FROM char_effects WHERE effectid = ?", (BATTLEFIELD_EFFECT,))
        for charid, power in cur.fetchall():
            if power in new_ids:
                cur.execute(
                    "UPDATE char_effects SET power = ? WHERE charid = ? AND effectid = ? AND power = ?",
                    (new_ids[power], charid, BATTLEFIELD_EFFECT, power),
                )

        # Two passes, so a renamed variable never lands on one that still has its old name.
        cur.execute("SELECT charid, varname FROM char_vars WHERE varname LIKE 'Battlefield[%'")
        renames = []
        for charid, varname in cur.fetchall():
            match = VAR_PATTERN.match(varname)
            if match and int(match[1]) in new_ids:
                renames.append((charid, varname, f"Battlefield[{new_ids[int(match[1])]}]{match[2]}"))

        for charid, varname, _ in renames:
            cur.execute("UPDATE char_vars SET varname = ? WHERE charid = ? AND varname = ?", ("~" + varname, charid, varname))
        for charid, varname, renamed in renames:
            cur.execute("UPDATE char_vars SET varname = ? WHERE charid = ? AND varname = ?", (renamed, charid, "~" + varname))

        cur.execute("DROP TABLE IF EXISTS `bcnm_records`;")
        db.commit()
    except mariadb.Error as err:
        print("Something went wrong: {}".format(err))
