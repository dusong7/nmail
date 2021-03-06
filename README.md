nmail - ncurses mail
====================

| **Linux + Mac** |
|---------|
| [![Build status](https://travis-ci.com/d99kris/nmail.svg?branch=master)](https://travis-ci.com/d99kris/nmail) |

nmail is a console-based email client for Linux and macOS with a user interface
similar to alpine / pine.

![screenshot](/doc/screenshot.png) 

Features
--------
- Support for IMAP and SMTP protocols
- Local cache using AES256-encrypted custom Maildir format
- Multi-threaded (email fetch and send done in background)
- Address book auto-generated based on email messages
- Viewing HTML emails
- Opening/viewing attachments in external program
- Simple setup wizard for Gmail and Outlook/Hotmail
- Familiar UI for alpine / pine users
- Compose message using external editor ($EDITOR)
- View message using external viewer ($PAGER)
- Saving and continuing draft messages

Planned features
----------------
- Email search

Not planned
-----------
- Multiple email accounts in a single session
- Special handling for Gmail labels
- Threaded view


Usage
=====

Usage:

    nmail [OPTION]

Command-line Options:

    -d, --confdir <DIR>
        use a different directory than ~/.nmail

    -e, --verbose
        enable verbose logging

    -ee, --extraverbose
        enable extra verbose logging

    -h, --help
        display this help and exit

    -o, --offline
        run in offline mode

    -s, --setup <SERV>
        setup wizard for specified service, supported services: gmail, outlook

    -v, --version
        output version information and exit

Configuration files:

    ~/.nmail/main.conf
        configures mail account and general setings.

    ~/.nmail/ui.conf
        customizes UI settings.

Examples:

    nmail -s gmail
        setup nmail for a gmail account


Supported Platforms
===================

nmail is developed and tested on Linux and macOS. Current version has been
tested on:

- macOS 10.14 Mojave
- Ubuntu 18.04 LTS


Build / Install
===============

Linux / Ubuntu
--------------

**Dependencies**

    sudo apt install git cmake libetpan-dev libssl-dev libncurses-dev help2man lynx

**Source**

    git clone https://github.com/d99kris/nmail && cd nmail

**Build**

    mkdir -p build && cd build && cmake .. && make -s

**Install**

    sudo make install

macOS
-----

**Dependencies**

    brew install cmake libetpan openssl ncurses help2man lynx

**Source**

    git clone https://github.com/d99kris/nmail && cd nmail

**Build**

    mkdir -p build && cd build && cmake .. && make -s

**Install**

    make install


Getting Started
===============

Gmail Setup
-----------
Use the setup wizard to set up nmail for the account. Example (replace
example@gmail.com with your actual gmail address):

    $ nmail -s gmail
    Email: example@gmail.com
    Name: Firstname Lastname
    Save password (y/n): y
    Password: 

Note: Refer to [Gmail Prerequisites](#gmail-prerequisites) for enabling gmail IMAP access.

Outlook (and Hotmail) Setup
---------------------------
Use the setup wizard to set up nmail for the account. Example (replace
example@hotmail.com with your actual outlook / hotmail address):

    $ nmail -s outlook
    Email: example@hotmail.com
    Name: Firstname Lastname
    Save password (y/n): y
    Password: 

Other Email Providers
---------------------
Run nmail once in order for it to automatically generate the default config
file:

    $ nmail

Then open the config file `~/.nmail/main.conf` in your favourite text editor
and fill out the required fields:

    address=example@example.com
    drafts=Drafts
    imap_host=imap.example.com
    imap_port=993
    inbox=INBOX
    name=Firstname Lastname
    smtp_host=smtp.example.com
    smtp_port=587
    trash=Trash
    user=example@example.com

Full example of a config file `~/.nmail/main.conf`:

    address=example@example.com
    cache_encrypt=1
    client_store_sent=0
    drafts=Drafts
    editor_cmd=
    ext_viewer_cmd=
    html_convert_cmd=
    imap_host=imap.example.com
    imap_port=993
    inbox=INBOX
    name=Firstname Lastname
    pager_cmd=
    prefetch_level=2
    save_pass=1
    sent=Sent
    smtp_host=smtp.example.com
    smtp_port=587
    smtp_user=
    trash=Trash
    user=example@example.com
    verbose_logging=0

### address

The from-address to use. Required for sending emails.

### cache_encrypt

Indicates whether nmail shall encrypt local message cache or not (default enabled).

### client_store_sent

This field should generally be left `0`. It indicates whether nmail shall upload
sent emails to configured `sent` folder. Many email service providers
(gmail, outlook, etc) do this on server side, so this should only be enabled if
emails sent using nmail do not automatically gets stored in the sent folder.

### drafts

Name of drafts folder - needed if using functionality to postpone email editing.

### editor_cmd

The field `editor_cmd` allows overriding which external editor to use when
composing / editing an email using an external editor (`Ctrl-E`). If not
specified, nmail will use the editor specified by the environment variable
`$EDITOR`. If `$EDITOR` is not set, nmail will use `nano`.

### ext_viewer_cmd

This field allows overriding the external viewer used when viewing email
parts and attachments. By default nmail uses `open` on macOS and `xdg-open`
on Linux.

### html_convert_cmd

This field allows customizing how nmail should convert HTML emails to text.
If not specified, nmail checks if `lynx`, `elinks` or `links` is available
on the system (in that order), and uses the first found. The exact command
used is one of:
- `lynx -assume_charset=utf-8 -display_charset=utf-8 -dump`
- `elinks -dump-charset utf-8 -dump`
- `links -codepage utf-8 -dump`

### imap_host

IMAP hostname / address. Required for fetching emails.

### imap_port

IMAP port. Required for fetching emails.

### inbox

IMAP inbox folder name. Required for nmail to open the proper default folder.

### name

Real name of sender. Recommended when sending emails.

### pager_cmd

The field `pager_cmd` allows overriding which external pager / text viewer to
use when viewing an email using an external pager (`Ctrl-E`). If not specified,
nmail will use the pager specified by the environment variable `$PAGER`.
If `$PAGER` is not set, nmail will use `less`.

### prefetch_level

Messages are pre-fetched from server based on the `prefetch_level` config
setting. The following levels are supported:

    0 = no pre-fetching, messages are retrieved when viewed
    1 = pre-fetching of currently selected message
    2 = pre-fetching of all messages in current folder view (default)
    3 = pre-fetching of all messages in all folders, i.e. full sync

### save_pass

Specified whether nmail shall store the password(s). Default 1 (enabled).

### sent

IMAP sent folder name. Used by nmail if `client_store_sent` is enabled to store
copies of outgoing emails. 

### smtp_host

SMTP hostname / address. Required for sending emails.

### smtp_port

SMTP port. Required for fetching emails. Default 587.

### smtp_user

The field `smtp_user` should generally be left blank, and only be specified in
case the email account has different username and password for sending emails
(or if one wants to use one email service provider for receiving and another
for sending emails). If not specified, the configured `user` field will be
used.

### trash

IMAP trash folder name. Needs to be specified in order to delete emails.

### user

Email account username for IMAP (and SMTP).

### verbose_logging

Allows forcing nmail to enable specified logging level:

    0 = info, warnings, errors (default)
    1 = debug (same as `-e`, `--verbose` - enable verbose logging)
    2 = trace (same as `-ee`, `--extraverbose` - enable extra verbose logging


Multiple Email Accounts
=======================

nmail does currently not support multiple email accounts (in a single session).
It is however possible to run multiple nmail instances in parallel with
different config directories (and thus different email accounts), but it will
be just that - multiple instances - each in its own terminal. To facilitate
such usage one can set up aliases for accessing different accounts, e.g.:

    alias gm='nmail -d ${HOME}/.nmail-gm' # gmail
    alias hm='nmail -d ${HOME}/.nmail-hm' # hotmail


Compose Editor
==============

The built-in email compose editor in nmail supports the following:

    Arrow keys     move the cursor
    Page Up/Down   move the cursor page up / down
    Ctrl-K         delete current line
    Ctrl-C         cancel message
    Ctrl-O         postpone message
    Ctrl-X         send message
    Ctrl-E         edit message in external editor
    Ctrl-T         to select, from address book / from file dialog
    Backspace      backspace
    Delete         delete
    Enter          new line

The email headers `To`, `Cc` and `Attchmnt` support comma-separated values, e.g.:

    To      : Alice <alice@example.com>, Bob <bob@example.com>
    Cc      : Chuck <chuck@example.com>, Dave <dave@example.com>
    Attchmnt: localpath.txt, /tmp/absolutepath.txt
    Subject : Hello world

Attachment paths may be local (just filename) or absolute (full path).


Troubleshooting
===============

If any issues are observed, try running nmail with verbose logging

    nmail --verbose

and provide a copy of ~/.nmail/log.txt when reporting the issue. The
preferred way of reporting issues and asking questions is by opening 
[a Github issue](https://github.com/d99kris/nmail/issues/new).

Verbose logging can also be enabled by setting `verbose_logging=1` in
`~/.nmail/main.conf`.

An extra verbose logging mode is also supported. It produces very large
log files with detailed information. The extra verbose logs typically
contain actual email contents. Review and edit such logs to remove any
private information before sharing. To enable extra verbose logging:

    nmail --extraverbose

Extra verbose logging can also be enabled by setting `verbose_logging=2` in
`~/.nmail/main.conf`.


Email List
==========
An email list is available for users to discuss nmail usage and related topics.
Feel free to [subscribe](http://www.freelists.org/list/nmail-users) and send
messages to [nmail-users@freelists.org](mailto:nmail-users@freelists.org).

Bug reports, feature requests and usage questions directed at the nmail
maintainer(s) should however be reported using
[Github issues](https://github.com/d99kris/nmail/issues/new) to ensure they
are properly tracked and get addressed.


Security
========

nmail caches data locally to improve performance. By default the cached
data is encrypted (`cache_encrypt=1` in main.conf). Messages are encrypted
using OpenSSL AES256-CBC with a key derived from a random salt and the
email account password. Folder names are hashed using SHA256 (thus
not encrypted).

Using the command line tool `openssl` it is possible to decrypt locally
cached messages / headers. Example (enter email account password at prompt):

    openssl enc -d -aes-256-cbc -md sha1 -in ~/.nmail/cache/imap/B5/152.eml

Storing the account password (`save_pass=1` in main.conf) is *not* secure.
While nmail encrypts the password, the key is trivial to determine from
the source code. Only store the password if measurements are taken to ensure
`~/.nmail/secret.conf` cannot by accessed by a third-party.


Configuration
=============

Aside from `main.conf` covered above, the following file can be used to
configure nmail.

~/.nmail/ui.conf
----------------
This configuration file controls the UI aspects of nmail. Default configuration
file:

    cancel_without_confirm=0
    compose_hardwrap=0
    help_enabled=1
    key_back=,
    key_cancel=KEY_CTRLC
    key_compose=c
    key_delete=d
    key_delete_line=KEY_CTRLK
    key_external_editor=KEY_CTRLE
    key_external_pager=KEY_CTRLE
    key_forward=f
    key_goto_folder=g
    key_move=m
    key_next_msg=n
    key_open=.
    key_othercmd_help=o
    key_postpone=KEY_CTRLO
    key_prev_msg=p
    key_quit=q
    key_refresh=l
    key_reply=r
    key_save_file=s
    key_send=KEY_CTRLX
    key_to_select=KEY_CTRLT
    key_toggle_text_html=t
    key_toggle_unread=u
    new_msg_bell=1
    persist_folder_filter=1
    plain_text=1
    postpone_without_confirm=0
    quit_without_confirm=1
    send_without_confirm=0
    show_embedded_images=1
    show_progress=1


Email Service Providers
=======================

Gmail Prerequisites
-------------------
Gmail prevents password-authenticated IMAP access by default.

In order to enable IMAP access go to the Gmail web interface - typically
[mail.google.com](https://mail.google.com) - and navigate to 
`Settings -> Forwarding and POP/IMAP -> IMAP access` and select: `Enable IMAP`

To enable password-authenticated IMAP access go to
[myaccount.google.com/security](https://myaccount.google.com/security) and under
`Less secure app access` click `Turn on access (not recommended)`.


Technical Details
=================

nmail is implemented in C++. Its source tree includes the source code of the
following third-party libraries:

- [apathy](https://github.com/dlecocq/apathy) - MIT License
- [cxx-prettyprint](https://github.com/louisdx/cxx-prettyprint) - Boost License


License
=======
nmail is distributed under the MIT license. See [LICENSE](/LICENSE) file.


Keywords
========
command line, console based, linux, macos, email client, ncurses, terminal,
alternative to alpine.

