###############################################
## line_status:
##		0		ready
##		10		ignore module rules
##		11		 module rules

function path_d2u(path)
{
	upath = path
	gsub("\\\\", "/", upath);
	upath = gensub(/^([a-zA-Z]):/, "/cygdrive/\\1", "g", upath);
	return upath
}
BEGIN {
	## the fullpath of the telelogic create.exe		
	TELELOGIC_CREATE="D:\\Program Files\\Telelogic\\Logiscope_6.5\\bin\\create.exe"
	## the fullpath of the telelogic batch.exe		
	TELELOGIC_BATCH="D:\\Program Files\\Telelogic\\Logiscope_6.5\\bin\\batch.exe"
	## where is the telelogic project place
	TELELOGIC_WORK_PATH="E:\\telelogic\\elephant\\"
	## the ignore file
	TELELOGIC_IGN_FILE="E:\\telelogic\\fullcode.ign"
	## the def file 
	TELELOGIC_DEF_FILE="E:\\telelogic\\fullcode.def"
	## the rule set
	TELELOGIC_RULESET="E:\\telelogic\\ALiRuleSet_v01.rst"

	# for private maintain
	line_status = 0

	#get the source path
	echo src_root_path
	gsub("/", "\\", src_root_path);
	if(0 == match(src_root_path, /^([a-zA-Z]):/))
	{
		src_root_path = gensub(/^\\cygdrive\\([a-zA-Z_])/, "\\1:", "g", src_root_path);
	}
	print src_root_path

}

/^[A-Z0-9_]+_[ \t]*:[ \t]*$/ {
	line_status = 10
	next
}

/^[A-Z0-9_]+[ \t]*:[ \t]*$/ {
	line_status = 11
	match($0, /(\w+)/, arr)
	mod_name =  arr[1]
	print $0
	next
}
/\tcd[ \t]+(.*);/ {
	if(11 == line_status) {
		match($0, /\tcd[ \t]+(.*);/, arr)
		mod_dir = arr[1]
		gsub("/", "\\", mod_dir)
		sub(/\\+$/, "", mod_dir)
		
		## general the telelogic command
		upath = TELELOGIC_WORK_PATH mod_dir
		upath = path_d2u(upath);
		print "\tmkdir -p " upath
		printf "\t\"%s\" -rule -lang c  -root \"%s\" -recurse -source \"*.c;*.h\" -rules \"%s\" -dial \"gnu\" -def \"%s\" -ign \"%s\" -repository \"%s\" \"%s\"\n", \
				path_d2u(TELELOGIC_CREATE), src_root_path "\\src\\" mod_dir, TELELOGIC_RULESET, TELELOGIC_DEF_FILE, TELELOGIC_IGN_FILE, \
				TELELOGIC_WORK_PATH "\\" mod_dir, TELELOGIC_WORK_PATH "\\" mod_dir "\\" mod_name ".ttp"
		
		printf "\t\"%s\" \"%s\" -o \"%s\"\n", path_d2u(TELELOGIC_BATCH), TELELOGIC_WORK_PATH "\\" mod_dir "\\" mod_name ".ttp", TELELOGIC_WORK_PATH "\\" mod_dir
				
	}
	if(line_status > 9) next

}
/\t/ {
	if(line_status > 9)
		next
}

{
	line_status = 0
	print $0

}
