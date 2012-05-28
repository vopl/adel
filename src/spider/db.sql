-- Database: spider

-- DROP DATABASE spider;

CREATE DATABASE spider
  WITH OWNER = spider
       ENCODING = 'UTF8'
       TABLESPACE = pg_default
       LC_COLLATE = 'C'
       LC_CTYPE = 'C'
       CONNECTION LIMIT = -1;

-- Table: host

-- DROP TABLE host;

CREATE TABLE host
(
  id bigserial NOT NULL,
  name character varying,
  atime timestamp without time zone,
  new_pages_count bigint NOT NULL DEFAULT 0,
  address character varying,
  CONSTRAINT host_pkey PRIMARY KEY (id )
)
WITH (
  OIDS=FALSE
);
ALTER TABLE host
  OWNER TO spider;

-- Index: host_name_idx

-- DROP INDEX host_name_idx;

CREATE INDEX host_name_idx
  ON host
  USING btree
  (name COLLATE pg_catalog."default" );

-- Index: host_new_pages_count_idx

-- DROP INDEX host_new_pages_count_idx;

CREATE INDEX host_new_pages_count_idx
  ON host
  USING btree
  (new_pages_count );

-- Table: page

-- DROP TABLE page;

CREATE TABLE page
(
  id bigserial NOT NULL,
  host_id bigint,
  url character varying,
  atime timestamp without time zone,
  status character varying,
  count bigint NOT NULL DEFAULT 0,
  get_time bigint,
  body_length bigint,
  headers character varying,
  CONSTRAINT page_pkey PRIMARY KEY (id ),
  CONSTRAINT hostref FOREIGN KEY (host_id)
      REFERENCES host (id) MATCH SIMPLE
      ON UPDATE NO ACTION ON DELETE NO ACTION
)
WITH (
  OIDS=FALSE
);
ALTER TABLE page
  OWNER TO spider;

-- Index: fki_hostref

-- DROP INDEX fki_hostref;

CREATE INDEX fki_hostref
  ON page
  USING btree
  (host_id );

-- Index: page_atime_idx

-- DROP INDEX page_atime_idx;

CREATE INDEX page_atime_idx
  ON page
  USING btree
  (atime );

-- Index: page_status_idx

-- DROP INDEX page_status_idx;

CREATE INDEX page_status_idx
  ON page
  USING btree
  (status COLLATE pg_catalog."default" );

-- Index: page_url_idx

-- DROP INDEX page_url_idx;

CREATE INDEX page_url_idx
  ON page
  USING btree
  (url COLLATE pg_catalog."default" );

-- Table: reference

-- DROP TABLE reference;

CREATE TABLE reference
(
  id bigserial NOT NULL,
  from_id bigint,
  to_id bigserial NOT NULL,
  CONSTRAINT reference_pkey PRIMARY KEY (id ),
  CONSTRAINT cnstr_from FOREIGN KEY (from_id)
      REFERENCES page (id) MATCH SIMPLE
      ON UPDATE NO ACTION ON DELETE NO ACTION
)
WITH (
  OIDS=FALSE
);
ALTER TABLE reference
  OWNER TO spider;

-- Index: fki_cnstr_from

-- DROP INDEX fki_cnstr_from;

CREATE INDEX fki_cnstr_from
  ON reference
  USING btree
  (from_id );

------------------------
INSERT INTO host (name, new_pages_count, atime) VALUES('127.0.0.1', 1, CURRENT_TIMESTAMP);
INSERT INTO page (host_id, url) VALUES(1, 'http://127.0.0.1/index.html');

