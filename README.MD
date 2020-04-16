![Fannst Banner](static/fannst-banner.png)

# FSMTP Server

Open source C++ SMTP Server

## Currently supporting

* ESMTP
* ESMTP With STARTTLS
* DKIM Signer

## Future support

* ESMTP Relaying
* ESMTP Auth
* ESMTP, with pipelining
* Web API
* Spam filters, using spamhaus
* DKIM Verification
* IMAP Server [ Not sure yet ]

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