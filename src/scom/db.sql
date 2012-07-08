
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
    --instance_id bigint NOT NULL REFERENCES instance(id) ON DELETE CASCADE ON UPDATE CASCADE,
    instance_id bigint,
    value varchar NOT NULL,
    kind_and_access int4 NOT NULL,
    kind_and_access_min int4 NOT NULL,
    kind_and_access_max int4 NOT NULL,
    amount int4 NOT NULL
);





--------------------------------------------------------
DROP TABLE IF EXISTS active_host CASCADE;
CREATE TABLE active_host
(
    id bigserial NOT NULL PRIMARY KEY,
    name varchar NOT NULL,
    atime timestamp without time zone
);

CREATE INDEX active_host_atime_idx
  ON active_host
  USING btree
  (atime );

CREATE INDEX active_host_name_idx
  ON active_host
  USING btree
  (name);






--------------------------------------------------------
DROP TABLE IF EXISTS page CASCADE;
CREATE TABLE page
(
    id bigserial NOT NULL PRIMARY KEY,
    --instance_id bigint NOT NULL REFERENCES instance(id) ON DELETE CASCADE ON UPDATE CASCADE,
    instance_id bigint,
    
    --active_host_id bigint NULL REFERENCES active_host(id) ON DELETE SET NULL ON UPDATE SET NULL,
    active_host_id bigint,
    
    uri varchar NOT NULL,
    access int4 NULL,

    status varchar,
    http_headers varchar,
    text varchar,
    ref_page_ids bytea,
    ip varchar,
    fetch_time int4,

    fetch_order bigint,
    atime timestamp without time zone
);

CREATE INDEX page_active_host_id_idx
  ON page
  USING btree
  (active_host_id );

CREATE INDEX page_instance_id_idx
  ON page
  USING btree
  (instance_id );

CREATE INDEX page_instance_id_uri_idx
  ON page
  USING btree
  (instance_id , uri);



CREATE OR REPLACE FUNCTION insertPageIfAbsent(instance_id_ bigint, uri_ varchar)
    RETURNS bigint
    LANGUAGE plpgsql
AS $$
DECLARE
	id_ bigint;
BEGIN
    LOOP
        -- first try to select
        SELECT id FROM page WHERE instance_id=instance_id_ AND uri=uri_ INTO id_;
        -- check if the row is found
        IF FOUND THEN
            RETURN id_;
        END IF;
        -- not found so insert the row
        BEGIN
            INSERT INTO page (instance_id, uri) VALUES(instance_id_, uri_) RETURNING id INTO id_;
	    IF FOUND THEN
		RETURN id_;
	    END IF;
            EXCEPTION WHEN unique_violation THEN
                -- do nothing and loop
        END;
    END LOOP;
END;
$$;
