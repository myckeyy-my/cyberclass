#!/usr/bin/env python3
"""Generate every password variant of a base word using optional l33t substitutions.

Usage:
    python leetspeak.py <base_password> <rules.json>

Each character may stay as it appears in the base password, or be replaced by
one of the strings listed for that letter. Matching is case-insensitive, so
both "e" and "E" use the "E" rule. A lowercase letter may also be capitalized,
so "r" can become "R" even when the JSON list does not contain "R". Characters
with no rule are left unchanged. One candidate is written per line on stdout.
"""

import json
import sys
from itertools import product


def load_rules(path):
    """Return a dict of uppercase letter -> list of substitute strings."""
    with open(path, encoding="utf-8") as handle:
        raw = json.load(handle)

    rules = {}
    for key, substitutes in raw.items():
        if not isinstance(substitutes, list):
            raise ValueError(f"rule for {key!r} must be a list")
        rules[str(key).upper()] = [str(item) for item in substitutes]
    return rules


def add_option(options, value):
    """Append a substitute unless this position already has it."""
    if value not in options:
        options.append(value)


def options_for(char, rules):
    """Original character, its uppercase form, then each JSON substitute."""
    options = [char]
    if char.isalpha():
        add_option(options, char.upper())
    for substitute in rules.get(char.upper(), []):
        add_option(options, substitute)
    return options


def main(argv):
    if len(argv) != 3:
        print(
            f"Usage: {argv[0]} <base_password> <rules.json>",
            file=sys.stderr,
        )
        return 1

    base_password = argv[1]
    try:
        rules = load_rules(argv[2])
    except (OSError, json.JSONDecodeError, ValueError) as exc:
        print(f"Error reading rules: {exc}", file=sys.stderr)
        return 1

    choices = [options_for(char, rules) for char in base_password]
    for combo in product(*choices):
        sys.stdout.write("".join(combo) + "\n")
    return 0


if __name__ == "__main__":
    sys.exit(main(sys.argv))
