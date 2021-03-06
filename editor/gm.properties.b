

# keywords4 are intended for syntax driven or custom highlighting
#keywords4.$(file.patterns.gm)=load

# Styles

# 0 - whitespace
# 1, 2, 3, 4 - comments:  /* */,  //,  /** */ or /*! */,  /+ +/
# 5 - number
# 6, 7, 8, 9 - keywords: standard, secondary, doc keywords, typedefs and aliases
# 10 - string
# 11 - string not closed
# 12 - char
# 13 - operator
# 14 - identifier
# 15, 16, 17 - Doc comments: line doc /// or //!, doc keyword, doc keyword err

style.gm.32=$(font.base)
style.gm.0=fore:#808080
style.gm.1=$(colour.code.comment.box),$(font.code.comment.box)
style.gm.2=$(colour.code.comment.line),$(font.code.comment.line)
style.gm.3=$(colour.code.comment.doc),$(font.code.comment.doc)
style.gm.4=$(colour.code.comment.nested),$(font.code.comment.nested)
style.gm.5=$(colour.number)
style.gm.6=$(colour.keyword),bold
style.gm.7=$(colour.keyword),bold
style.gm.8=$(colour.keyword),bold
style.gm.9=$(colour.keyword),bold
style.gm.10=$(colour.string)
style.gm.11=fore:#000000,$(font.monospace),back:#E0C0E0,eolfilled
style.gm.12=$(colour.char)
style.gm.13=$(colour.operator),bold
style.gm.14=
style.gm.15=$(colour.code.comment.doc),$(font.code.comment.doc)
style.gm.16=fore:#B00040
style.gm.17=fore:#804020,$(font.code.comment.doc)

# breaces must be operator style to allow matching
braces.gm.style=13

command.name.0.*.gm=Format
command.is.filter.0.*.gm=1
command.quiet.0.*.gm=1
command.shortcut.0.*.gm=F9
command.save.before.0.*.gm=1
#command.0.*.gm=$(SciteDefaultHome)\..\bin\format.bat $(FileNameExt)
command.0.*.gm=$(SciteDefaultHome)\AStyle --style=attach --add-one-line-brackets --keep-one-line-blocks --keep-one-line-statements $(FileNameExt)

command.name.1.*.gm=Package
command.is.filter.1.*.gm=0
command.quiet.1.*.gm=0
command.shortcut.1.*.gm=Ctrl+F12
command.save.before.1.*.gm=1
command.subsystem.1.*.gm=2
command.1.*.gm="$(SciteDefaultHome)\..\scripts\package.gm" "$(FilePath)"

command.go.*.gm=$(SciteDefaultHome)\..\bin\xstart.exe $(FileNameExt)
command.help.*.gm=explorer "file://$(SciteDefaultHome)\help.htm#$(CurrentWord)"
command.help.subsystem.*.gm=1
