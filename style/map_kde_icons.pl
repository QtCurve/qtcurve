#This perl script is called by the QtCurve Gtk2 theme, it is not intended to be useful by itself.
# (C) Craig Drummond, 2007 - 2009
# Release uneder the GPL, v2 or later.
#
# Usage perl map_kde_icons.pl <icon map file> <kde prefix> <kde version> <small toolbar size> <toolbar size> <dnd size> <btn size> <menu size> <dialog size> <icons map file version>
#@iconSizes = ( 22,                  32,           22,        16,         16,                  48           );
# KDE Uses 32x32 for dialogs, and 16x16 for buttons
@iconSizes = ( $ARGV[3],            $ARGV[4],             $ARGV[5], $ARGV[6],     $ARGV[7],   $ARGV[8]     );
@gtk       = ( "gtk-small-toolbar", "gtk-large-toolbar", "gtk-dnd", "gtk-button", "gtk-menu", "gtk-dialog" );
$numSizes=$#iconSizes+1;

printf "#%s %02X%02X%02X%02X%02X%02X%02X\n", $ARGV[9], $ARGV[2], $ARGV[3], $ARGV[4], $ARGV[5], $ARGV[6], $ARGV[7], $ARGV[8];
printf "#This file is created, and used by, QtCurve. Alterations may be overwritten.\n";
print "gtk-icon-sizes=\"gtk-small-toolbar=$ARGV[3],$ARGV[3]:gtk-large-toolbar=$ARGV[4],$ARGV[4]:";
print "gtk-dnd=$ARGV[5],$ARGV[5]:gtk-button=$ARGV[6],$ARGV[6]:gtk-menu=$ARGV[7],$ARGV[7]:gtk-dialog=$ARGV[8],$ARGV[8]\"\n";
#print "gtk-dnd=$ARGV[5],$ARGV[5]:gtk-button=$ARGV[6],$ARGV[6]:gtk-menu=$ARGV[7],$ARGV[7]\"\n";

if ($ARGV[1])
{
    $base=$ARGV[1];
}

if($ARGV[2] == "3")
{
    $base=join("", $base, "/crystalsvg/");
}
else
{
    $base=join("", $base, "/oxygen/");
}

open(icons, "$ARGV[0]") || die "Could not open \"$ARGV[0]\"\n";

sub checkSize
{
    $fname=join("", $_[0], $_[1], "x", $_[1], "/", $_[2]);
    if (open(tst, $fname))
    {
        close(tst);
        $fname;
    }
    else
    {
        "";
    }
}

sub printSize
{
    if($_[0])
    {
        print "\t\t{ \"$_[1]x$_[1]/$_[2]\", *, *, \"$_[3]\" },\n";
        $_[4]--;
    }
}

print "\nstyle \"KDE$ARGV[2]-icons\"\n{\n";

while($entry=<icons>)
{
    @iconMap=split(/ /, $entry);

    for($i=0; $i<$#iconMap; $i++)
    {
        $iconMap[$i+1]=~s/\n//;
    }

    if($iconMap[1] == /WHAT/)
    {
        $got=0;
        $use=0;
        for($i=0; $i<$#iconMap && $use == 0; $i++)
        {
            for($index=0; $index<$numSizes; $index++)
            {
                $files[$index]=checkSize($base, $iconSizes[$index], $iconMap[$i+1]);
                if($files[$index])
                {
                    $got++;
                }
            }

            if($got)
            {
                $use=$i+1;
            }
        }

        if($got)
        {
            print "\tstock[\"$iconMap[0]\"]={\n";
            if($got > 1)
            {
                for($index=0; $index<$numSizes; $index++)
                {
                    $got=printSize($files[$index], $iconSizes[$index], $iconMap[$use], $gtk[$index], $got);
                }
            }
            $found=0;
            for($index=0; $index<$numSizes && !$found; $index++)
            {
                if($files[$index])
                {
                    print "\t\t{ \"$iconSizes[$index]x$iconSizes[$index]/$iconMap[$use]\" }\n";
                    $found=1;
                }
            }
            print "\t}\n";
        }
        else
        {
            print "#\tstock[\"$iconMap[0]\"]=<No matching KDE icon>\n";
        }
    }
}
print "}\nclass \"*\" style \"KDE$ARGV[2]-icons\"\n";

close(icons);
