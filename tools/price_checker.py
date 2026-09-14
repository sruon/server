import re
import os

logging = False
sell_prices = dict()
errors = list()

server_dir_path = os.path.normpath(
    os.path.dirname(os.path.dirname(os.path.abspath(__file__)))
)


def from_server_path(path):
    return os.path.normpath(os.path.join(server_dir_path, path))


def load_items(items_path):
    """Reads each item's key, id, auction category and base sell price off data/items.yaml without a YAML parser."""
    items = {}
    in_items = False
    key = None

    with open(items_path, "r", encoding="utf-8") as f:
        for line in f:
            if not in_items:
                in_items = line.rstrip() == "items:"
                continue

            match = re.match(r"^  ([a-z0-9_]+):\s*$", line)
            if match:
                key = match.group(1)
                items[key] = {}
                continue

            match = re.match(r"^    (id|auction|base_sell):\s*(\S+)", line)
            if match and key is not None:
                items[key][match.group(1)] = match.group(2)

    return items


items = load_items(from_server_path("data/items.yaml"))
item_enums = {"xi.item.NONE": 0}
for key, fields in items.items():
    item_enums[f"xi.item.{key.upper()}"] = int(fields["id"])


def log(message, *args):
    if logging:
        length = len(args)
        if length == 2:
            print(message.format(args[0], args[1]))
        elif length == 3:
            print(message.format(args[0], args[1], args[2]))


def process_npc(path):
    with open(path, mode="r", errors="ignore") as npc_file:
        in_stock = False
        brace_depth = 0

        for line in npc_file:
            stripped = line.strip()

            if not in_stock and "local stock" in stripped:
                in_stock = True
                continue

            if in_stock:
                brace_depth += stripped.count("{")
                brace_depth -= stripped.count("}")

                if brace_depth < 1:
                    continue

                # Match inner stock lines like: { item_id, buy_price, [ignored], ... }
                match = re.match(r"\{\s*(.+?)\s*\}", stripped)
                if not match:
                    continue

                # Extract and split the inner values
                values = [v.strip() for v in match.group(1).split(",") if v.strip()]
                if len(values) < 2:
                    continue  # malformed or incomplete line

                item_id_raw, buy_price = values[0], values[1]

                # Resolve enum or raw
                if item_id_raw.startswith("xi.item."):
                    resolved = item_enums.get(item_id_raw)
                    if resolved is None:
                        errors.append(
                            f"Unknown item enum {item_id_raw} found in script {path}."
                        )
                        continue  # enum not found
                    item_id = resolved
                elif item_id_raw.isdigit():
                    item_id = int(item_id_raw)
                else:
                    errors.append(f"Unknown item type found in script {path}.")
                    continue  # unhandled type

                log(f"Found item {item_id} with buy price {buy_price}")

                if item_id in sell_prices and sell_prices[item_id] > int(buy_price):
                    errors.append(
                        f"Found item {item_id_raw} with buy price {buy_price} which is lower than sell price {sell_prices[item_id]} in script {path}."
                    )


# items kept off the auction house that sell to shops for gil
for fields in items.values():
    if "auction" not in fields and "base_sell" in fields:
        sell_prices[int(fields["id"])] = int(fields["base_sell"])

        log(f"Added item {fields['id']} with base sell price {fields['base_sell']}")

# iterate over npcs in ../scripts/zones/.../npcs/*.lua
with os.scandir(from_server_path("scripts/zones")) as iterator:
    for entry in iterator:
        if entry.is_dir():
            npc_path = os.path.join(entry.path, "npcs")
            # iterate over npcs folder if it exists
            if os.path.exists(npc_path):
                with os.scandir(npc_path) as npc_iterator:
                    for npc in npc_iterator:
                        if npc.is_file():
                            process_npc(npc.path)

for error in errors:
    print(error)
if len(errors) > 0:
    print(f"Found {len(errors)} errors.")
