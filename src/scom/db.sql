
--------------------------------------------------------
DROP TABLE IF EXISTS instance CASCADE;
CREATE TABLE instance
(
    id bigserial NOT NULL PRIMARY KEY,
    password varchar NOT NULL,
    stage int4 NOT NULL DEFAULT 0,
    ctime TIMESTAMP WITHOUT TIME ZONE,
    atime TIMESTAMP WITHOUT TIME ZONE,
    dtime TIMESTAMP WITHOUT TIME ZONE
);

--------------------------------------------------------
DROP TABLE IF EXISTS page_rule CASCADE;
CREATE TABLE page_rule
(
    id bigserial NOT NULL PRIMARY KEY,
    instance_id bigint NOT NULL REFERENCES instance(id),
    is_src boolean NOT NULL,
    base_uri varchar NOT NULL,
    kind_and_access int4 NOT NULL,
    kind_and_access_min int4 NOT NULL,
    kind_and_access_max int4 NOT NULL
);

--------------------------------------------------------
DROP TABLE IF EXISTS page CASCADE;
CREATE TABLE page
(
    id bigserial NOT NULL PRIMARY KEY,
    instance_id bigint NOT NULL REFERENCES instance(id),
    is_src boolean NOT NULL,
    uri varchar NOT NULL,
    is_allowed boolean NOT NULL,
    rule_id bigint NOT NULL REFERENCES page_rule(id),

    http_status varchar,
    http_headers varchar,
    http_body varchar,

    atime timestamp without time zone
);
