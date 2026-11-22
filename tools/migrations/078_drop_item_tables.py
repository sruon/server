import mariadb

DROPPED_TABLES = [
    "item_basic",
    "item_equipment",
    "item_furnishing",
    "item_latents",
    "item_mods",
    "item_mods_pet",
    "item_puppet",
    "item_usable",
    "item_weapon",
]


def migration_name():
    return "Drop item tables data/items.yaml replaced"


def check_preconditions(cur):
    return


def needs_to_run(cur):
    for table in DROPPED_TABLES:
        cur.execute(f"SHOW TABLES LIKE '{table}';")
        if cur.fetchone():
            return True

    return False


def migrate(cur, db):
    try:
        for table in DROPPED_TABLES:
            cur.execute(f"DROP TABLE IF EXISTS `{table}`;")

        db.commit()
    except mariadb.Error as err:
        print("Something went wrong: {}".format(err))
