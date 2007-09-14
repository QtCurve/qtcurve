#!/bin/bash

#
# Taken from http://rignesnet.tzo.com/articles/mailto_helper.html
# ...and modified a little...
#

MAILTO_URL="$@"

app=`kreadconfig --file emaildefaults --group PROFILE_Default --key EmailClient --default kmail`

if [ "$app" == "" ] ; then
    app=kmail
fi

case `basename $app` in
    "mozilla-thunderbird" | "thunderbird" | "evolution")
        $app "$MAILTO_URL"
        ;;
    "kmailservice" | "kmail")
        kmailservice "$MAILTO_URL"
        ;;
    *)
        #Strip off the protocol
        MAIL_DATA=$(echo "$MAILTO_URL" | /bin/sed -s 's/^mailto://I')

        #Get Recipient and strip it off
        RECIPIENT=$(echo "$MAIL_DATA" | cut -d? -f1 -)
        MAIL_DATA=$(echo "$MAIL_DATA" | /bin/sed -s "s/^$RECIPIENT//")

        #Get Subject,BCC, and CC
        SUBJECT=$(echo "$MAIL_DATA" | \
        /bin/sed -s 's/.*?subject=//I' | /bin/sed -s 's/?.*//')
        BCC=$(echo "$MAIL_DATA" | /bin/sed -s 's/.*?bcc=//I' | \
        /bin/sed -s 's/?.*//')
        CC=$(echo "$MAIL_DATA" | /bin/sed -s 's/.*?cc=//I' | \
        /bin/sed -s 's/?.*//')

        $app "$RECIPIENT" -b "$BCC" -c "$CC" -s "$SUBJECT"
        ;;
esac

