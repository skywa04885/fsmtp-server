![Fannst Banner](static/fannst-banner.png)

# ABANDONDED !!!
New project: https://github.com/skywa04885/fsmtp-v2

# FSMTP Server

Open source C++ SMTP Server, developed by [Fannst Software](https://software.fannst.nl)

## Example conversation
* R: Client received
* S: Client sent

```bash
$ telnet 127.0.0.1 25

--- Server connects and assigns new thread ---

R: 220 smtp.fannst.nl - ESMTP Ready
S: HELO [127.0.0.1]

R: 250-smtp.fannst.nl at your service, [127.0.0.1]
   250-STARTTLS
   250-HELP
   250 AUTH DIGEST-MD5
S: STARTTLS
R: 220 Ok: continue - fsmtp

--- TLS Connecting ---

S: HELO [127.0.0.1]
R: 250-smtp.fannst.nl at your service, [127.0.0.1]
   250-STARTTLS
   250-HELP
   250 AUTH DIGEST-MD5 PLAIN LOGIN

S: MAIL FROM: <luke.rieff@gmail.com>
R: 250 Ok: proceed - fsmtp

S: RCPT TO: <luke@fannst.nl>
R: 250 ok: proceed - fsmtp

S: DATA
R: 354 End data with <CR><LF>.<CR><LF> - fsmtp

S: From: Luke A.C.A. Rieff <luke.rieff@gmail.com>
   To: Luke Rieff <luke@fannst.nl>
   Subject: Hello World
   Message-ID: <518e39bc-73ab-e294-0e4c-e81ff755a1d5@gmail.com>
   MIME-Version: 1.0
   Content-Type: text/plain; charset=utf-8; format=flowed;

   test

   .

R: 250 Ok: queued as 0 - fsmtp
S: QUIT
R: 221 Goodbye ! - fsmtp

--- Server terminates connection ---
```

## Example generated MIME Message, by FSMTP

```
MIME-Version: 1.0
X-Author: Luke A.C.A. Rieff
Subject: Hello World
Return-Path: <webmaster@fannst.nl>
Message-ID: <yqjw8yx8-nuzi-g01o-pdawqw5jwsql@fannst.nl>
Content-Type: multipart/alternative; boundary="--_CujdngdVAfkhJkZgyyd0tGPFx1C"
Date: Tue, 21 Apr 2020 09:32:34 +0200
From: Webmaster <webmaster@fannst.nl>
To: Test <luke.rieff@gmail.com>
DKIM-Signature: d=fannst.nl; s=default; t=1587454355; x=1587460355; c=relaxed/relaxed; 
    q=dns/txt; v=1; a=rsa-sha256; h=mime-version:subject:message-id:date:from:to; 
    bh=hjcJt7MMn2BmW1t5n4zsq+OXU8iXDGbAcM3rzZaDveY=; 
    b=Ce1gEZrFXaLzxRdWxFfKItZH3J42xn/GWXe4gb3oAcenWXEDCBFcyEdiZVchg2mV9
     Gra7+3rkv5DDE4IIhzoCiV9p6M5ts5bp4nX19uEYVym6Cej6FEuuxZ7vfN9RyTLtsvL
     Fk9DrlUuhFZjHQCVDVW94BhmldsWQWC2Joga7e3HXJ2ujuzJ6Us+FPLhOJG7ueAdup5
     dvnGZxJUu8MxG5n+UYb5B12puXWiDesSTqjsOSscL5F8FBsvDWPXIT9axxIwyO+MFfy
     su21yHu1soDi3X1lV+gatYqdvLdUa8ifG+lUJZTihIZJnDe8obuIwvAz2O0QeZGmodB
     yXSNxKzPA==

----_CujdngdVAfkhJkZgyyd0tGPFx1C
Content-Type: text/html
Content-Transfer-Encoding: quoted-printable

<h1>Hello World</h1><p>Kaas</p>
----_CujdngdVAfkhJkZgyyd0tGPFx1C
Content-Type: text/plain
Content-Transfer-Encoding: 8bit

Hello World
Kaas
----_CujdngdVAfkhJkZgyyd0tGPFx1C--
```

## Tested with, and worked

* Outlook
* Google Mail
* Yahoo Mail

## Currently supporting

* ESMTP
* ESMTP With STARTTLS
* DKIM Signer
* ESMTP Relaying
* ESMTP Auth ( Plain only )

## Future support

* ESMTP, with pipelining
* Web API
* Spam filters, using spamhaus
* DKIM Verification
* IMAP Server \[ Not sure yet \]

## Dependencies

* [Mongoose](https://github.com/cesanta/mongoose/)

## How to build ?

```bash
sudo chmod +x ./build.sh
sudo bash ./build.sh
```

## Developers

|Name          |Github                                         |Website                                  |
|--------------|-----------------------------------------------|-----------------------------------------|
|Luke Rieff    |[skywa04885](https://github.com/skywa04885)    |[me.fannst.nl](https://me.fannst.nl)     |

Want to become an developer for Fannst Software ? 
Feel free to contact us for more information, keep in mind that you will be doing
this as a volunteer, and will not get payed. Click [Here](https://software.fannst.nl) to get in touch.

---

