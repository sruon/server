import mariadb

TABLES = [
    "fishing_area",
    "fishing_bait",
    "fishing_bait_affinity",
    "fishing_catch",
    "fishing_fish",
    "fishing_group",
    "fishing_mob",
    "fishing_rod",
    "fishing_zone",
]


def migration_name():
    return "Drop legacy static fishing tables"


def check_preconditions(cur):
    return


def needs_to_run(cur):
    for table in TABLES:
        cur.execute("SHOW TABLES LIKE '{}'".format(table))
        if cur.fetchone():
            return True
    return False


def migrate(cur, db):
    try:
        for table in TABLES:
            cur.execute("DROP TABLE IF EXISTS `{}`;".format(table))
        db.commit()
    except mariadb.Error as err:
        print("Something went wrong: {}".format(err))
