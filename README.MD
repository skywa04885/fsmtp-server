![Fannst Banner](static/fannst-banner.png)

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
R: 220 Ok: continue

--- TLS Connecting ---

S: HELO [127.0.0.1]
R: 250-smtp.fannst.nl at your service, [127.0.0.1]
   250-STARTTLS
   250-HELP
   250 AUTH DIGEST-MD5 PLAIN LOGIN

S: MAIL FROM: <luke.rieff@gmail.com>
R: 250 Ok: proceed

S: RCPT TO: <luke@fannst.nl>
R: 250 ok: proceed

S: DATA
R: 354 End data with <CR><LF>.<CR><LF>

S: From: Luke A.C.A. Rieff <luke.rieff@gmail.com>
   To: Luke Rieff <luke@fannst.nl>
   Subject: Hello World
   Message-ID: <518e39bc-73ab-e294-0e4c-e81ff755a1d5@gmail.com>
   MIME-Version: 1.0
   Content-Type: text/plain; charset=utf-8; format=flowed;

   test

   .

R: 250 Ok: queued as 0
S: QUIT
R: 221 Goodbye !

--- Server terminates connection ---
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
mkdir build
cd build

cmake ..
make -j [CPUS]
sudo ./fsmtp_server
```

## Developers

Only me LOL !

|Name          |Github                                         |Website                                  |
|--------------|-----------------------------------------------|-----------------------------------------|
|Luke Rieff    |[skywa04885](https://github.com/skywa04885)    |[me.fannst.nl](https://me.fannst.nl)     |