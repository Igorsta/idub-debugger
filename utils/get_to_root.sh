#!/bin/bash
set -e

CURRENT_DIR="$(cd "$(dirname "${BASH_SOURCE[0]}")" && pwd)"

find_project_root() {
    local dir="$1"
    while [[ "$dir" != "/" ]]; do
        if [[ -d "$dir/.git" ]]; then
            echo "$dir"
            return 0
        fi
        dir="$(dirname "$dir")"
    done
    return 1  # Not found
}

PROJECT_ROOT="$(find_project_root "$CURRENT_DIR")"

if [[ -z "$PROJECT_ROOT" ]]; then
    echo "❌ Could not locate project root!"
    exit 1
fi

echo "Project root detected at: $PROJECT_ROOT"

cd "$PROJECT_ROOT"