import mariadb

TABLES = [
    "despoil_effects",
    "gardening_results",
    "cheat_types",
]


def migration_name():
    return "Drop legacy despoil_effects, gardening_results and cheat_types tables"


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
