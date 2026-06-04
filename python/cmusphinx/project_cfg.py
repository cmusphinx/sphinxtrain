"""Load sphinx_train.resolved.json produced by sphinxtrain run or resolve-config."""

from __future__ import annotations

import json
from pathlib import Path


class ConfigError(Exception):
    pass


def load_resolved(etc_dir: Path) -> dict:
    path = etc_dir / "sphinx_train.resolved.json"
    if not path.is_file():
        raise ConfigError(
            "missing etc/sphinx_train.resolved.json; run sphinxtrain run "
            "(or resolve-config) from the project directory"
        )
    with open(path, encoding="utf-8") as fh:
        return json.load(fh)


def variable(doc: dict, name: str) -> str:
    value = doc.get("variables", {}).get(name)
    if value is None:
        raise ConfigError(f"missing variables.{name} in sphinx_train.resolved.json")
    return str(value)


def derived(doc: dict, key: str) -> str:
    value = doc.get("derived", {}).get(key)
    if value is None:
        raise ConfigError(f"missing derived.{key} in sphinx_train.resolved.json")
    return str(value)
