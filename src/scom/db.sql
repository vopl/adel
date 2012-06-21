
--------------------------------------------------------
DROP TABLE IF EXISTS instance CASCADE;
CREATE TABLE instance
(
    id bigserial NOT NULL PRIMARY KEY,
    password varchar NOT NULL,
    stage int4 NOT NULL DEFAULT 0,
    is_started boolean NOT NULL DEFAULT false,
    ctime TIMESTAMP WITHOUT TIME ZONE,
    atime TIMESTAMP WITHOUT TIME ZONE,
    dtime TIMESTAMP WITHOUT TIME ZONE
);

--------------------------------------------------------
DROP TABLE IF EXISTS page_rule CASCADE;
CREATE TABLE page_rule
(
    id bigserial NOT NULL PRIMARY KEY,
    instance_id bigint NOT NULL REFERENCES instance(id) ON DELETE CASCADE ON UPDATE CASCADE,
    value varchar NOT NULL,
    kind_and_access int4 NOT NULL,
    kind_and_access_min int4 NOT NULL,
    kind_and_access_max int4 NOT NULL,
    max_amount int4 NOT NULL
);





--------------------------------------------------------
DROP TABLE IF EXISTS active_host CASCADE;
CREATE TABLE active_host
(
    id bigserial NOT NULL PRIMARY KEY,
    name varchar NOT NULL,
    atime timestamp without time zone
);

--------------------------------------------------------
DROP TABLE IF EXISTS page CASCADE;
CREATE TABLE page
(
    id bigserial NOT NULL PRIMARY KEY,
    instance_id bigint NOT NULL REFERENCES instance(id) ON DELETE CASCADE ON UPDATE CASCADE,
    active_host_id bigint NULL REFERENCES active_host(id) ON DELETE SET NULL ON UPDATE SET NULL,
    uri varchar NOT NULL,
    is_allowed boolean,

    http_status varchar,
    http_headers varchar,
    http_body varchar,

    atime timestamp without time zone
);

--------------------------------------------------------
DROP TABLE IF EXISTS page_ref CASCADE;
CREATE TABLE page_ref
(
    referrer_page_id bigint NOT NULL REFERENCES page(id) ON DELETE CASCADE ON UPDATE CASCADE,
    referenced_page_id bigint NOT NULL REFERENCES page(id) ON DELETE CASCADE ON UPDATE CASCADE
);


