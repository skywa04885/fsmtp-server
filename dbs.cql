-- Creates the keyspace
create keyspace if not exists fmail with replication = {
    'class': 'SimpleStrategy',
    'replication_factor': 3
};

-- Uses the keyspace
use fmail;

-- Creates the users table
create table if not exists users (
    u_fullname varchar,
    u_password varchar,
    u_domain varchar,
    u_username varchar,
    u_uuid uuid,
    u_description text,
    u_picture text,
    u_birth_date bigint,
    u_creation_date bigint,
    u_bucket bigint,
    primary key((u_bucket), u_domain, u_uuid)
) with clustering order by (u_domain DESC, u_uuid DESC);

-- Drops users table
drop table users;

-- Creates the short access user database
create table if not exists users_quick_access (
    u_domain varchar,
    u_username varchar,
    u_uuid uuid,
    u_bucket bigint,
    u_password varchar,
    primary key((u_domain), u_username)
) with clustering order by (u_username DESC);

-- Drops short access tables
drop table users_quick_access;

-- Drops the users table
drop table users;

-- Email system types
create type if not exists email_address (
    e_name varchar,
    e_address varchar
);

create type if not exists email_header (
    e_key varchar,
    e_value varchar
);

create type if not exists email_content_section (
    e_content text,
    e_type int,
    e_full_headers frozen<list<email_header>>,
    e_index int
);

-- Creates the email table
create table if not exists inbox_emails (
    m_transport_to email_address,
    m_transport_from email_address,
    m_subject text,
    m_message_id varchar,
    m_date bigint,
    m_boundary varchar,
    m_content_Type int,
    m_timestamp bigint,
    m_receive_timestamp bigint,
    m_from frozen<list<email_address>>,
    m_to frozen<list<email_address>>,
    m_full_headers frozen<list<email_header>>,
    m_content frozen<list<email_content_section>>,
    m_bucket bigint,
    m_uuid uuid,
    m_user_uuid uuid,
    primary key((m_bucket), m_user_uuid, m_uuid)
) with clustering order by(m_user_uuid DESC, m_uuid DESC);

-- Creates the quick access email table
create table if not exists inbox_emails_quick_access (
    m_bucket bigint,
    m_user_uuid uuid,
    m_uuid uuid,
    m_receive_timestamp bigint,
    m_subject varchar,
    m_content_snippet varchar,
    m_from_str varchar,
    m_user_domain varchar,
    primary key((m_user_domain), m_user_uuid, m_receive_timestamp)
) with clustering order by(m_user_uuid DESC, m_receive_timestamp DESC);
