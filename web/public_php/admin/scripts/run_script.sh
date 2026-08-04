#!/bin/bash
# Only start one of the known companion scripts under this directory, and
# only with argv that look like name=value pairs. Never pass the raw "$*"
# of a web request through to php.
set -e
cd "$(dirname "$0")"

script="$1"
shift || true

case "$script" in
	restart_sequence.php) ;;
	*)
		echo "Access denied" >&2
		exit 1
		;;
esac

args=()
for a in "$@"; do
	case "$a" in
		[A-Za-z_]*=*) args+=("$a") ;;
		*)
			echo "Access denied" >&2
			exit 1
			;;
	esac
done

nohup php "$script" "${args[@]}" >/dev/null 2>&1 &
