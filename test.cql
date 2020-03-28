use fmail;

-- Inserts the users
insert into users (
    u_fullname, u_password, u_domain,
    u_username, u_uuid, u_description,
    u_picture, u_birth_date, u_creation_date,
    u_bucket
) values (
    'Fannst Admin', 'd1c1f5beadea331c17a913f1c9f0a84434514eafcdc140fa5a03f88abbfa9920', 'fannst.nl',
    'admin', now(), 'I am the all mighty Skywa04885 ;>',
    'https://cd1.fannst.nl/stc/fannst.nl/admin.jpg', toUnixTimestamp(now()), toUnixTimestamp(now()),
    400
);

insert into users (
    u_fullname, u_password, u_domain,
    u_username, u_uuid, u_description,
    u_picture, u_birth_date, u_creation_date,
    u_bucket
) values (
    'Luke Rieff', 'd1c1f5beadea331c17a913f1c9f0a84434514eafcdc140fa5a03f88abbfa9920', 'fannst.nl',
    'luke', now(), 'I am the all mighty personal account of Skywa04885 ;>',
    'https://cd1.fannst.nl/stc/fannst.nl/luke.jpg', toUnixTimestamp(now()), toUnixTimestamp(now()),
    400
);

insert into users_quick_access (

)

-- Gets the users
select * from users;