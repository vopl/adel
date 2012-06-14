--
-- PostgreSQL database dump
--

-- Dumped from database version 9.1.2
-- Dumped by pg_dump version 9.1.2
-- Started on 2012-06-12 21:10:07

SET statement_timeout = 0;
SET client_encoding = 'UTF8';
SET standard_conforming_strings = on;
SET check_function_bodies = false;
SET client_min_messages = warning;


SET search_path = public, pg_catalog;

SET default_with_oids = false;

--
-- TOC entry 161 (class 1259 OID 1782574)
-- Dependencies: 5
-- Name: page; Type: TABLE; Schema: public; Owner: -
--

DROP TABLE IF EXISTS page CASCADE;
-- Table: page

-- DROP TABLE page;

CREATE TABLE page
(
  id bigserial NOT NULL,
  site_id bigint,
  uri character varying,
  body_length bigint,
  headers character varying,
  status character varying,
  get_time integer,
  CONSTRAINT page_pkey PRIMARY KEY (id )
)
WITH (
  OIDS=FALSE
);
ALTER TABLE page
  OWNER TO spider;

-- Index: page_status_idx

-- DROP INDEX page_status_idx;

CREATE INDEX page_status_idx
  ON page
  USING btree
  (status COLLATE pg_catalog."default" );

-- Index: page_status_idx1

-- DROP INDEX page_status_idx1;

CREATE INDEX page_status_idx1
  ON page
  USING btree
  (status COLLATE pg_catalog."default" )
  WHERE status IS NULL;



--
-- TOC entry 163 (class 1259 OID 1782582)
-- Dependencies: 5
-- Name: reference; Type: TABLE; Schema: public; Owner: -
--

DROP TABLE IF EXISTS reference CASCADE;
-- Table: reference

-- DROP TABLE reference;

CREATE TABLE reference
(
  id bigserial NOT NULL,
  from_id bigint,
  to_id bigint NOT NULL
)
WITH (
  OIDS=FALSE
);
ALTER TABLE reference
  OWNER TO spider;

--
-- TOC entry 165 (class 1259 OID 1782587)
-- Dependencies: 1884 1885 1886 1887 1888 1889 1890 1891 1892 5
-- Name: site; Type: TABLE; Schema: public; Owner: -
--

DROP TABLE IF EXISTS site CASCADE;
-- Table: site

-- DROP TABLE site;

CREATE TABLE site
(
  id bigserial NOT NULL,
  name character varying NOT NULL,
  address character varying,
  priority double precision NOT NULL DEFAULT 1.0,
  amount_ref_incoming integer NOT NULL DEFAULT 0,
  amount_ref_outgoing integer NOT NULL DEFAULT 0,
  amount_page_all integer NOT NULL DEFAULT 0,
  amount_page_new integer NOT NULL DEFAULT 0,
  amount_page_update integer NOT NULL DEFAULT 0,
  amount_page_dead integer NOT NULL DEFAULT 0,
  time_per_page interval NOT NULL DEFAULT '00:00:10'::interval,
  time_access timestamp without time zone NOT NULL DEFAULT now(),
  CONSTRAINT site_pkey PRIMARY KEY (id )
)
WITH (
  OIDS=FALSE
);
ALTER TABLE site
  OWNER TO spider;

-- Index: site_expr_idx

-- DROP INDEX site_expr_idx;

CREATE INDEX site_expr_idx
  ON site
  USING btree
  ((time_access + time_per_page) );

-- Index: site_time_access_idx

-- DROP INDEX site_time_access_idx;

CREATE INDEX site_time_access_idx
  ON site
  USING btree
  (time_access );



--
-- TOC entry 167 (class 1259 OID 1782604)
-- Dependencies: 5
-- Name: word2; Type: TABLE; Schema: public; Owner: -
--

DROP TABLE IF EXISTS word2 CASCADE;
-- Table: word2

-- DROP TABLE word2;

CREATE TABLE word2
(
  id bigserial NOT NULL,
  word1 integer,
  word2 integer
)
WITH (
  OIDS=FALSE
);
ALTER TABLE word2
  OWNER TO spider;

-- Index: word2_word1_word2_idx

-- DROP INDEX word2_word1_word2_idx;

CREATE INDEX word2_word1_word2_idx
  ON word2
  USING btree
  (word1 , word2 );



--
-- TOC entry 169 (class 1259 OID 1782609)
-- Dependencies: 5
-- Name: word2_to_page; Type: TABLE; Schema: public; Owner: -
--

DROP TABLE IF EXISTS word2_to_page CASCADE;
-- Table: word2_to_page

-- DROP TABLE word2_to_page;

CREATE TABLE word2_to_page
(
  word2_id bigint,
  page_id bigint
)
WITH (
  OIDS=FALSE
);
ALTER TABLE word2_to_page
  OWNER TO spider;

  -- Table: word2_to_page_tmp

-- DROP TABLE word2_to_page_tmp;

CREATE TABLE word2_to_page_tmp
(
  page_id bigint,
  word1 integer,
  word2 integer
)
WITH (
  OIDS=FALSE
);
ALTER TABLE word2_to_page_tmp
  OWNER TO spider;

-- Trigger: on_delete_word2_tmp on word2_to_page_tmp

-- DROP TRIGGER on_delete_word2_tmp ON word2_to_page_tmp;

CREATE TRIGGER on_delete_word2_tmp
  BEFORE DELETE
  ON word2_to_page_tmp
  FOR EACH ROW
  EXECUTE PROCEDURE on_delete_word2_tmp();



--
-- TOC entry 170 (class 1259 OID 1782612)
-- Dependencies: 5
-- Name: word3; Type: TABLE; Schema: public; Owner: -
--

DROP TABLE IF EXISTS word3 CASCADE;
-- Table: word3

-- DROP TABLE word3;

CREATE TABLE word3
(
  id bigserial NOT NULL,
  word1 integer,
  word2 integer,
  word3 integer
)
WITH (
  OIDS=FALSE
);
ALTER TABLE word3
  OWNER TO spider;

-- Index: word3_word1_word2_word3_idx

-- DROP INDEX word3_word1_word2_word3_idx;

CREATE INDEX word3_word1_word2_word3_idx
  ON word3
  USING btree
  (word1 , word2 , word3 );



--
-- TOC entry 172 (class 1259 OID 1782617)
-- Dependencies: 5
-- Name: word3_to_page; Type: TABLE; Schema: public; Owner: -
--

DROP TABLE IF EXISTS word3_to_page CASCADE;
-- Table: word3_to_page

-- DROP TABLE word3_to_page;

CREATE TABLE word3_to_page
(
  word3_id bigint,
  page_id bigint
)
WITH (
  OIDS=FALSE
);
ALTER TABLE word3_to_page
  OWNER TO spider;


-- Table: word3_to_page_tmp

-- DROP TABLE word3_to_page_tmp;

CREATE TABLE word3_to_page_tmp
(
  page_id bigint,
  word1 integer,
  word2 integer,
  word3 integer
)
WITH (
  OIDS=FALSE
);
ALTER TABLE word3_to_page_tmp
  OWNER TO spider;

-- Trigger: on_delete_word3_tmp on word3_to_page_tmp

-- DROP TRIGGER on_delete_word3_tmp ON word3_to_page_tmp;

CREATE TRIGGER on_delete_word3_tmp
  BEFORE DELETE
  ON word3_to_page_tmp
  FOR EACH ROW
  EXECUTE PROCEDURE on_delete_word3_tmp();



--
-- TOC entry 1900 (class 0 OID 1782574)
-- Dependencies: 161
-- Data for Name: page; Type: TABLE DATA; Schema: public; Owner: -
--

INSERT INTO page (id, site_id, uri, body_length, headers, status) VALUES (1, 1, 'http://127.0.0.1:8080/index.html', NULL, NULL, NULL);


--
-- TOC entry 1901 (class 0 OID 1782582)
-- Dependencies: 163
-- Data for Name: reference; Type: TABLE DATA; Schema: public; Owner: -
--



--
-- TOC entry 1902 (class 0 OID 1782587)
-- Dependencies: 165
-- Data for Name: site; Type: TABLE DATA; Schema: public; Owner: -
--

INSERT INTO site (id, name, address, priority, amount_ref_incoming, amount_ref_outgoing, amount_page_all, amount_page_new, amount_page_update, amount_page_dead, time_per_page, time_access) VALUES (1, '127.0.0.1:8080', '127.0.0.1', 1, 0, 0, 1, 1, 0, 0, '00:00:10', '2012-06-12 21:06:36.558');


--
-- TOC entry 1903 (class 0 OID 1782604)
-- Dependencies: 167
-- Data for Name: word2; Type: TABLE DATA; Schema: public; Owner: -
--



--
-- TOC entry 1904 (class 0 OID 1782609)
-- Dependencies: 169
-- Data for Name: word2_to_page; Type: TABLE DATA; Schema: public; Owner: -
--



--
-- TOC entry 1905 (class 0 OID 1782612)
-- Dependencies: 170
-- Data for Name: word3; Type: TABLE DATA; Schema: public; Owner: -
--



--
-- TOC entry 1906 (class 0 OID 1782617)
-- Dependencies: 172
-- Data for Name: word3_to_page; Type: TABLE DATA; Schema: public; Owner: -
--






-- Function: on_delete_word2_tmp()

-- DROP FUNCTION on_delete_word2_tmp();

CREATE OR REPLACE FUNCTION on_delete_word2_tmp()
  RETURNS trigger AS
$BODY$
DECLARE
  mID bigint;
  cID CURSOR IS SELECT id FROM word2 WHERE word1=OLD.word1 AND word2=OLD.word2;
BEGIN
  OPEN cid;
  FETCH cID INTO mID;
  CLOSE cid;
  IF NOT FOUND
  THEN
    BEGIN
        INSERT INTO word2 (word1,word2) VALUES (OLD.word1,OLD.word2) RETURNING id INTO mID;
    EXCEPTION WHEN unique_violation THEN
      OPEN cid; 
      FETCH cID INTO mID;
      CLOSE cid;
    END;
  END IF;

  INSERT INTO word2_to_page (word2_id, page_id) VALUES (mID,OLD.page_id);
  RETURN OLD;
END;
$BODY$
  LANGUAGE plpgsql VOLATILE
  COST 100;
ALTER FUNCTION on_delete_word2_tmp()
  OWNER TO postgres;

DROP TRIGGER IF EXISTS on_delete_word2_tmp ON word2_to_page_tmp;
CREATE TRIGGER on_delete_word2_tmp BEFORE DELETE
    ON word2_to_page_tmp FOR EACH ROW
    EXECUTE PROCEDURE on_delete_word2_tmp();





-- Function: on_delete_word3_tmp()

-- DROP FUNCTION on_delete_word3_tmp();

CREATE OR REPLACE FUNCTION on_delete_word3_tmp()
  RETURNS trigger AS
$BODY$
DECLARE
  mID bigint;
  cID CURSOR IS SELECT id FROM word3 WHERE word1=OLD.word1 AND word2=OLD.word2 AND word3=OLD.word2;
BEGIN
  OPEN cid;
  FETCH cID INTO mID;
  CLOSE cid;
  IF NOT FOUND
  THEN
    BEGIN
        INSERT INTO word2 (word1,word2,word3) VALUES (OLD.word1,OLD.word2,OLD.word3) RETURNING id INTO mID;
    EXCEPTION WHEN unique_violation THEN
      OPEN cid; 
      FETCH cID INTO mID;
      CLOSE cid;
    END;
  END IF;

  INSERT INTO word3_to_page (word3_id, page_id) VALUES (mID,OLD.page_id);
  RETURN OLD;
END;
$BODY$
  LANGUAGE plpgsql VOLATILE
  COST 100;
ALTER FUNCTION on_delete_word3_tmp()
  OWNER TO postgres;

DROP TRIGGER IF EXISTS on_delete_word3_tmp ON word3_to_page_tmp;
CREATE TRIGGER on_delete_word3_tmp BEFORE DELETE
    ON word3_to_page_tmp FOR EACH ROW
    EXECUTE PROCEDURE on_delete_word3_tmp();

