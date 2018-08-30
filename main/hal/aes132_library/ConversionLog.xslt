<?xml version="1.0" encoding="utf-8"?>
<xsl:stylesheet version="1.0" xmlns:xsl="http://www.w3.org/1999/XSL/Transform" xmlns:msxsl="urn:schemas-microsoft-com:xslt" >
  <xsl:template match="/">
    <html>
      <head>
        <style type="text/css">
          .infotable
          {
          TEXT-ALIGN: LEFT;
          BACKGROUND-COLOR: #f0f0e0;
          BORDER-BOTTOM: #ffffff 0px solid;
          BORDER-COLLAPSE: collapse;
          BORDER-LEFT: #ffffff 0px solid;
          BORDER-RIGHT: #ffffff 0px solid;
          BORDER-TOP: #ffffff 0px solid;
          FONT-SIZE: 70%;
          MARGIN-LEFT: 10px
          }
          BODY
          {
          BACKGROUND-COLOR: white;
          FONT-FAMILY: "Verdana", sans-serif;
          FONT-SIZE: 100%;
          MARGIN-LEFT: 0px;
          MARGIN-TOP: 0px
          }
          P1
          {
          FONT-FAMILY: "Verdana", sans-serif;
          FONT-SIZE: 70%;
          LINE-HEIGHT: 12pt;
          MARGIN-BOTTOM: 0px;
          MARGIN-LEFT: 10px;
          MARGIN-TOP: 10px
          }
          .note
          {
          BACKGROUND-COLOR:  #ffffff;
          COLOR: #336699;
          FONT-FAMILY: "Verdana", sans-serif;
          FONT-SIZE: 70%;
          MARGIN-BOTTOM: 0px;
          MARGIN-LEFT: 0px;
          MARGIN-TOP: 0px;
          PADDING-LEFT: 10px
          }
          .header
          {
          BACKGROUND-COLOR: #cecf9c;
          BORDER-BOTTOM: #ffffff 1px solid;
          BORDER-LEFT: #ffffff 1px solid;
          BORDER-RIGHT: #ffffff 1px solid;
          BORDER-TOP: #ffffff 1px solid;
          COLOR: #000000;
          FONT-WEIGHT: bold
          }
          .header1
          {
          BACKGROUND-COLOR: #E7D788;
          BORDER-BOTTOM: #ffffff 1px solid;
          BORDER-LEFT: #ffffff 1px solid;
          BORDER-RIGHT: #ffffff 1px solid;
          BORDER-TOP: #ffffff 1px solid;
          COLOR: #000000;
          FONT-WEIGHT: bold
          }
          .header2
          {
          BACKGROUND-COLOR: #E7cece;
          BORDER-BOTTOM: #ffffff 1px solid;
          BORDER-LEFT: #ffffff 1px solid;
          BORDER-RIGHT: #ffffff 1px solid;
          BORDER-TOP: #ffffff 1px solid;
          COLOR: #000000;
          FONT-WEIGHT: bold
          }


          .content
          {
          BACKGROUND-COLOR: #e7e7ce;
          BORDER-BOTTOM: #ffffff 1px solid;
          BORDER-LEFT: #ffffff 1px solid;
          BORDER-RIGHT: #ffffff 1px solid;
          BORDER-TOP: #ffffff 1px solid;
          PADDING-LEFT:3px
          }
          .content1
          {
          BACKGROUND-COLOR:#F8EDB8;
          BORDER-BOTTOM: #ffffff 1px solid;
          BORDER-LEFT: #ffffff 1px solid;
          BORDER-RIGHT: #ffffff 1px solid;
          BORDER-TOP: #ffffff 1px solid;
          PADDING-LEFT:3px
          }
          .content3
          {
          BACKGROUND-COLOR: #f3e7e7;
          BORDER-BOTTOM: #ffffff 1px solid;
          BORDER-LEFT: #ffffff 1px solid;
          BORDER-RIGHT: #ffffff 1px solid;
          BORDER-TOP: #ffffff 1px solid;
          PADDING-LEFT:3px
          }
          H1
          {
          BACKGROUND-COLOR: #003366;
          BORDER-BOTTOM: #336699 6px solid;
          COLOR: #ffffff;
          FONT-SIZE: 130%;
          FONT-WEIGHT: normal;
          MARGIN: 0em 0em 0em -20px;
          PADDING-BOTTOM: 8px;
          PADDING-LEFT: 30px;
          PADDING-TOP: 16px

          }
          H2
          {
          COLOR: #000000;
          FONT-SIZE: 80%;
          FONT-WEIGHT: bold;
          MARGIN-BOTTOM: 3px;
          MARGIN-LEFT: 10px;
          MARGIN-TOP: 20px;
          PADDING-LEFT: 0px
          }

          H4
          {
          COLOR: #ECEBEB;
          COLOR: #ffffff;
          FONT-WEIGHT: bold
          }
          th
          {
          width: 30%;
          }
          td
          {
          width: 30%;
          }
          .errortext
          {
          COLOR: #FF0000;

          }

        </style>
      </head>


      <body>

        <h1>
          <xsl:variable name="conversion_element" select="conversion" />
          <xsl:value-of select="$conversion_element/@conversiontitle"/>
          <br/>
          <h4>
            <xsl:value-of select="$conversion_element/@subtitle1"/>&#32;<xsl:value-of select="conversion/revert/@versiontext"/>&#32;
            <xsl:value-of select="$conversion_element/@subtitle2"/>&#32;<xsl:value-of select="conversion/update/@versiontext"/>
          </h4>
        </h1>
        <br/>

        <p1>
          <span class="note">
            <xsl:variable name="conversion_element1" select="conversion" />
            <b >Time of Conversion: &#32;&#32;</b>  <xsl:value-of select="$conversion_element1/@date"/>,&#32;&#32;<xsl:value-of select="$conversion_element1/@time"/>
          </span>
        </p1>
        <br/>

        <p1>
            <h2> Status </h2>

          <table cellpadding="2" cellspacing="0" width="98%" border="1" bordercolor="white" class="infotable">
            <tr>
              <td nowrap="1" class ="header">Timestamp</td>
              <td nowrap="1" class ="header">Message</td>
            </tr>
            <xsl:for-each select="//general/log">
              <tr>
                <td>
                  <xsl:value-of select="@timestamp"/>
                </td>
                <td>
                  <xsl:value-of select="message"/>
                </td>
              </tr>
            </xsl:for-each>
          </table>
        </p1>
        <br/>

        <!-- Logging for missingmodules-->

        <xsl:variable name="missingmodules_element" select="conversion/missingmodules" />
        <xsl:if test="$missingmodules_element/@name='Missing Modules'">
          <h2>
            <xsl:value-of select="$missingmodules_element/@name"/>&#32;
            in<xsl:variable name="conversion_element3" select="conversion" />
            &#32;<xsl:value-of select="conversion/update/@versiontext"/>
          </h2>
          <!-- Loop Through Missing modules -->
          <xsl:for-each select="conversion/missingmodules">
            <xsl:variable name ="missingmodules1_element" select="."/>
            <table cellpadding="2" cellspacing="0" width="98%" border="1" bordercolor="white" class="infotable">
              <tr>
                <th nowrap="1" class ="header2">Name</th>
              </tr>

              <!--ACTIONS-->
              <xsl:for-each select="child::*">
                <tr>
                  <td class = "content3">
                    <p class="errortext">
                      <xsl:value-of select="@name"/>
                    </p>
                  </td>
                </tr>
              </xsl:for-each>
              <!--End of actions-->
            </table>
          </xsl:for-each>
        </xsl:if>
        <br/>
        
         <!--Logging for revert--> 
        <h2>
          <xsl:variable name="revert_element" select="conversion/revert" />
          <xsl:value-of select="$revert_element/@description"/>&#32;
          <xsl:value-of select="$revert_element/@versiontext"/>
        </h2>

        <!-- Loop Through Module Info -->
        <xsl:for-each select="conversion/revert/moduleinfo">
          <xsl:variable name ="revert_moduleinfo_element" select="."/>
          <table cellpadding="2" cellspacing="0" width="98%" border="1" bordercolor="white" class="infotable">
            <tr>
              <th>
                Module Name : <xsl:value-of select="$revert_moduleinfo_element/@name"/>
              </th>
            </tr>
            <tr>
              <td nowrap="1" class ="header">Description</td>
              <td nowrap="1" class ="header">Result</td>
              <td nowrap="1" class ="header">Message</td>
            </tr>

            <!--ACTIONS-->
            <xsl:for-each select="child::*">
              <tr>
                <td class = "content">
                  <xsl:choose>
                    <xsl:when test="@status = 'Fail'">
                      <p class="errortext">
                        <xsl:value-of select="@description"/>
                      </p>
                    </xsl:when>
                    <xsl:otherwise>
                      <xsl:value-of select="@description"/>
                    </xsl:otherwise>
                  </xsl:choose>
                </td>
                <td class = "content">
                  <xsl:choose>
                    <xsl:when test="@status = 'Fail'">
                      <p class="errortext">
                        <xsl:value-of select="@result"/>
                      </p>
                    </xsl:when>
                    <xsl:otherwise>
                      <xsl:value-of select="@result"/>
                    </xsl:otherwise>
                  </xsl:choose>
                </td>
                <td class = "content">
                  <xsl:choose>
                    <xsl:when test="@status = 'Fail'">
                      <p class="errortext">
                        <xsl:value-of select="@error"/>
                      </p>
                    </xsl:when>
                    <xsl:otherwise>
                      <xsl:value-of select="@error"/>
                    </xsl:otherwise>
                  </xsl:choose>
                </td>
              </tr>

            </xsl:for-each>
            <!--End of actions-->

          </table>
        </xsl:for-each>

        <br/>
        <!-- Logging for update -->
        <h2>
          <xsl:variable name="update_element" select="conversion/update"/>
          <xsl:value-of select="$update_element/@description"/>&#32;
          <xsl:value-of select="$update_element/@versiontext"/>
        </h2>
        <!-- Loop Through Module Info -->
        <xsl:for-each select="conversion/update/moduleinfo">
          <xsl:variable name ="update_moduleinfo_element" select="."/>
          <table cellpadding="2" cellspacing="0" width="98%" border="1" bordercolor="white" class="infotable">
            <tr>
              <th>
                Module Name : <xsl:value-of select="$update_moduleinfo_element/@name"/>
              </th>
            </tr>
            <tr>
              <td nowrap="1" class ="header1">Description</td>
              <td nowrap="1" class ="header1">Result</td>
              <td nowrap="1" class ="header1">Message</td>
            </tr>

            <!--ACTIONS-->
            <xsl:for-each select="child::*">
              <tr>
                <td class = "content1">
                  <xsl:choose>
                    <xsl:when test="@status = 'Fail'">
                      <p class="errortext">
                        <xsl:value-of select="@description"/>
                      </p>
                    </xsl:when>
                    <xsl:otherwise>
                      <xsl:value-of select="@description"/>
                    </xsl:otherwise>
                  </xsl:choose>
                </td>
                <td class = "content1">
                  <xsl:choose>
                    <xsl:when test="@status = 'Fail'">
                      <p class="errortext">
                        <xsl:value-of select="@result"/>
                      </p>
                    </xsl:when>
                    <xsl:otherwise>
                      <xsl:value-of select="@result"/>
                    </xsl:otherwise>
                  </xsl:choose>
                </td>
                <td class = "content1">
                  <xsl:choose>
                    <xsl:when test="@status = 'Fail'">
                      <p class="errortext">
                        <xsl:value-of select="@error"/>
                      </p>
                    </xsl:when>
                    <xsl:otherwise>
                      <xsl:value-of select="@error"/>
                    </xsl:otherwise>
                  </xsl:choose>
                </td>
              </tr>

            </xsl:for-each>
            <!--End of actions-->
          </table>
        </xsl:for-each>

      </body>
    </html>
  </xsl:template>
</xsl:stylesheet>