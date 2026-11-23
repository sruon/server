SET FOREIGN_KEY_CHECKS=0;

-- ----------------------------
-- Table structure for `battlefield_records`
-- ----------------------------
DROP TABLE IF EXISTS `battlefield_records`;
CREATE TABLE `battlefield_records` (
  `bcnmId` smallint(5) unsigned NOT NULL,
  `fastestName` varchar(15) NOT NULL,
  `fastestPartySize` tinyint(2) unsigned NOT NULL,
  `fastestTime` int(10) unsigned NOT NULL,
  PRIMARY KEY (`bcnmId`)
) ENGINE=InnoDB DEFAULT CHARSET=utf8mb4 COLLATE=utf8mb4_general_ci;
