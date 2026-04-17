# bash completion for poedit

_poedit()
{
	local cur prev words cword
	_init_completion || return

	if [[ $cur == -* ]]; then
		COMPREPLY=($(compgen -W '--verbose --quiet --help --version' -- "$cur"))
		return
	fi

	_filedir
}

complete -F _poedit poedit
