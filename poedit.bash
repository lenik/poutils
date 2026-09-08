# bash completion for poedit

_poedit()
{
	local cur prev words cword
	_init_completion || return

	case "$prev" in
	-i|--input|-p|--po-dir)
		_filedir
		return
		;;
	-l|--langs)
		return
		;;
	esac

	if [[ $cur == -* ]]; then
		COMPREPLY=($(compgen -W '-i --input -l --langs -p --po-dir --dry-run -v --verbose -q --quiet -h --help --version' -- "$cur"))
		return
	fi

	_filedir
}

complete -F _poedit poedit
