<?
include_once("../classes/main.class.php");

class Help extends Main
{
	var $className = "������";
	var $classVersion = "1.0";
	var $fileName = "help.php";
	var $sortNum = 9;
	
	function displayContent()
	{
		?>
				
		<br>
		<a href="#1" class="t1" style="text-decoration: none"><strong>1. ������������</strong></a><br>
		<a href="#2" class="t1" style="text-decoration: none"><strong>2. �������</strong></a><br>
		<a href="#3" class="t1" style="text-decoration: none"><strong>3. �������</strong></a><br>
		<a href="#4" class="t1" style="text-decoration: none"><strong>4. ����������</strong></a><br>
		<a href="#5" class="t1" style="text-decoration: none"><strong>5. ������</strong></a><br>
		<a href="#6" class="t1" style="text-decoration: none"><strong>6. ���������</strong></a><br>
		<a href="#7" class="t1" style="text-decoration: none"><strong>7. ��������</strong></a><br>
	
		<br><br>
		<a name="1" id="1"><strong>1. ������������</strong></a><br><br>
		<div style="margin-left: 20px">
		<strong>1.1 ������ �������������</strong><br><br>
		<ul type="square">
			<li>��� ������������ ������������ � ������.
			<li>���������� ����� ������������.
			<li>��� �������� ����� ������ ������� &laquo;������� ����� ������&raquo; � ������� �������� (�� ����� 100 ��������).
			<li>�� ������ ������� ����������� �� ������� (��. ������ "��������"), ��������������� �� ���� ������������� � ���� ������ 
			<li>��� �� �� ������ ������� ��� ��������������� �������� ������. ��� �������� ������ ����� ������� ��� ������������, �������� � ���.
		</ul>
		<br><br>

		<strong>1.2 ������������</strong><br><br>
		<ul type="square">
			<li>��� �������� ������ ������������ ������� &laquo;������� ������ ������������&raquo;. �������:
			<ol>
				<li>��� (�� ����� 100 ��������)
				<li>������
				<li>IP-����� � ������� 192.168.0.�
				<li>MAC-����� � ������� XX:XX:XX:XX:XX:XX
				<li>��� �����������
				<li>�����. ������ ������: "-1" - ��� ������; "100" - 100 ����; "100K" - 100 ��������; "100M" - 100 �������� 
				<li>����������� (�������������)
				<li>�������� �����������. � �������� ������� ��. � ������� "��������"
			</ol>
			<li>��� �� �� ������: 
			<ol>
				<li>������� ��� ��������������� ��������� ������������.
				<li>������������� ������������. ������ � �������� ����� ������.
			</ol>
			<br><font color="#999999">����������: MAC-����� �������� �������������� ����������</font>
		</ul>
		<br><br>
		
		<strong>1.3 �������</strong><br><br>
		<ul type="square">
			<li>������� ��������� ���������� ������ ������������ � �������� �� ������ ����� � ����� �������� (��. ������ "�������")
			<li>��� �������� ������������ ��� ���� ������������� ��������� �������, �������������� ������ ������
			<li>��� �������� ������ ������� ������� &laquo;�������� �������&raquo;. �������:
			<ol>
				<li>��������
				<li>��� �����������
				<li>�����. ������ ������: "-1" - ��� ������; "100" - 100 ����; "100K" - 100 ��������; "100M" - 100 �������� 
			</ol>
			<li>��� �������� ������ ������ ������� &laquo;�������� ������ ������&raquo;. �������:
			<ol>
				<li>������
				<li>��� ������������� ������� ����������� �� ������. ������� ������ � �����. ������ ������: "-1" - ��� ������; "100" - 100 ����; "100K" - 100 ��������; "100M" - 100 ��������
				<li>����� ����� ������������� ��� ������� ������
			</ol>
			<li>��� �� �� ������: 
			<ol>
				<li>������� ��� ��������������� ��������� �������
				<li>������� ��� ��������������� ������ ������
				<li>������������� (���������) �/��� ��������� ������� �� ����� ����������. ��� ����� ���������� ������ &laquo;������������� �������&raquo;. ��������������� ������� ������������
		</ul>
		</div>
		<br><br>
		
		<a name="2" id="2"><strong>2. �������</strong></a><br><br>
		<div style="margin-left: 20px">
		<strong>2.1 ������ ��������</strong><br><br>
		<ul type="square">
			<li>��� ���� �������� ������������ � ������.
			<li>���������� ����� ������������.
			<li>��� �������� ����� ������ ������� &laquo;������� ����� ������&raquo; � ������� �������� (�� ����� 100 ��������)
			<li>�� ������ ������� ����������� �� ������� (��. � ������ "��������"), ��������������� �� ��� ���� �������� � ���� ������ 
			<li>��� �� �� ������ ������� ��� ��������������� �������� ������. ��� �������� ������ ����� ������� ��� ���� ��������, �������� � ���
		</ul>
		<br><font color="#999999">����������: ����� ������� ������ ������ ������</font>
		<br><br>
		
		<strong>2.2 ���� ��������</strong><br><br>
		<ul type="square">
			<li>��� �������� ������ ���� ������� &laquo;������� ����� ��� ��������&raquo;. �������:
			<ol>
				<li>�������� (�� ����� 100 ��������)
				<li>������
				<li>�����������
				<li>��������
				<li>����/����� 
				<li>�����
				<li>����������� �� ������� (��. � ������ "��������") ��� ����� ��������
			</ol>
			<li>��� �� �� ������ ������� ��� ��������������� ��������� ���� ��������
			</ol>
		</ul>
		<br><font color="#999999">����������: ���������� ������� ������� ���� �� ������������ � �������� �������������</font>
		
		</div>
		<br><br>
		
		
		<a name="3" id="3"><strong>3. �������</strong></a><br><br>
		<div style="margin-left: 20px">
		<ul type="square">
			<li>������� ��������� �������� ���� ����� ������� �� ���� ������ ������� (�������� �������� �����). ���������� �� �������� ������� �� ���� �������������
			<li>��� �������� ������ ������� ������� &laquo;������� ����� ������&raquo;. �������:
			<ol>
				<li>��������
				<li>��������
				<li>���� (0 - 65535)
			</ol>
			<li>��� �� �� ������ ������� ������ ��� ��������������� ��� ���������
		</ul>
		</div>
		<br><br>
		
		<a name="4" id="4"><strong>4. ����������</strong></a><br><br>
	
		<div style="margin-left: 20px">
		<strong>4.1 �����</strong><br><br>
		<ul type="square">
			<li>����� ���������� �� ���� �������������
		</ul>
		<br><br>

		<strong>4.2 �� �������</strong><br><br>
		<ul type="square">
			<li>���������� �� �������������, �������� �� �������
		</ul>
		<br><br>
		
		<strong>4.3 �� ��������</strong><br><br>
		<ul type="square">
			<li>���������� �� �������������, �� ��������� ���������� �� ������������. ����� �������� ���������� �� IP-������ � �� �������� (�� ������, �� ������� ����� � �� �������) - ��� ����� ���������� �������� �� ��������� ���������������� �������
		</ul>
		<br><br>

		<strong>4.4 ���������</strong><br><br>
		<ul type="square">
			<li>��������� ���������� �� ���������� ������������. ���������� ���������� �� ��������� �������� � �� ������� ������
		</ul>
		</div>
		<br><br>

		<a name="5" id="5"><strong>5. ������</strong></a><br><br>
		<div style="margin-left: 20px">
		<ul type="square">
			<li>������ ��������� ����������� ���������� �� ������������� �� ������������ �������� �������. ������ ����� ���� ������������:
			<ol>
				<li>�� IP-�������
				<li>�� URL-�������
				<li>�� �������
				<li>�� ��������
				<li>�� �������� ������������� (������ �������� ������������� �� ��������� ������) 
				<li>������� ����� (������ ������������� �� ���� �� ��������� ������)
			</ol>
			<br>
			<li>��� ������������ ������ �������� ������������, ������� ������, ��� ������ (����� ����������� �� ���������� ������� ����� X ��) � ������� &laquo;������������&raquo;.
			<br><br><font color="#999999">
			����������: � ������ ������ &laquo;��� ������������&raquo; ������������ ������ ����� ������ ��������������� �����
			<br>����������: � ������ ������������ ������ �� ������� ����� �������� ��������� ����� �� ������� ������ ���
			<br>����������: � ������ ������������ ������ �� URL-�������
				<div style="margin-left: 20px">
				�� ���� �������������: ����� �������� ��������� ����� �� �������������, ���������� ������ ������<br>
				�� ���������� ������������: ����� �������� ��������� ����� �� ��������� ��� ������� �������
				</div>
			<br>����������: � ������ ������������ ������ �� ��������
				<div style="margin-left: 20px">
				�� ���� �������������: ����� �������� ��������� ����� �� ������� ������� � ������������ ��������� ������ �������������, ����������� ���� ������<br>
				�� ���������� ������������: ����� �������� ��������� ����� ������� �������, ������������� ������������
				</div>
			</font>
			<br>
			<li>����� �� �������� ��������� ����������� ������ �������� ���� ��������� ��������
			<li>��� ������������ ������ ������� ������ � ������� &laquo;������������&raquo;
			<br><br><font color="#999999">����������: ����� ����� �������� ��������� ����� �� �������������, ������������ ������ ���������� ������</font>
		</ul>
		</div>
		<br><br>
		
		<a name="6" id="6"><strong>6. ���������</strong></a><br><br>
		<div style="margin-left: 20px">
		<strong>6.1 ��������� MySQL</strong><br><br>
		<ul type="square">
			<li>��������� ��� ������� MySQL
		</ul>
		<br><br>

		<strong>6.2 ��������� - �������������</strong><br><br>
		<ul type="square">
			<li>��������� ��� ����� � ���������� ��������������
		</ul>
		<br><br>

		<strong>6.3 ��������� - ������������</strong><br><br>
		<ul type="square">
			<li>��������� ��� ����� � ���������� �������������
		</ul>
		<br><br>

		<strong>6.4 ���</strong><br><br>
		<ul type="square">
			<li>��� �� ��������� ��������� � ���������������� ����������
		</ul>
		</div>
		<br><br>

		<a name="7" id="7"><strong>7. ��������</strong></a><br><br>
		<div style="margin-left: 20px">
		<strong>7.1 ����������� �������</strong><br><br>
		<ul type="square">
			<li>������ �������� ��������� ������������ ������ � �������� � ����������� �� ��� ������ � �������. ��� �������� ������� &laquo;������� ��������&raquo; � ������� ������ ������� ��� ������� ��� ������ � ������� &laquo;��:��-��:��&raquo;. 
			��� ������� ������� � ������������ ���� ����� ������� &laquo;00:00-00:00&raquo;. ����� ����� ����� ��������� �������� ��� ������������ � &laquo;�������������� �������������&raquo;
		</ul>
		<br><br>

		<strong>7.2 ����������� �������</strong><br><br>
		<ul type="square">
			<li>������ �������� ��������� ������������ ������ � ������������ �������� ����������� �� ��������� ����� � URL-������. ������� 2 ������ ������: ������������ ����� ���� ��������� ������ ��������� ������� ���� �� ����� ����� ������ �� ������ ������.
			��� ����� ��� �������� �������� ������� &laquo;��������� ��� ����� ������ ��������&raquo; � &laquo;��������� ��� ����� ������ ��������&raquo; ��������������. ����� ����� ����� ��������� �������� ��� ������������ � &laquo;�������������� �������������&raquo;
			<li>��� ������ �������� �������� �������� ������� �� ���������� �����. ������ ������ ������ ���������� � ��������� ������
		</ul>
		</div>
		<br><br>
		
		
		<br><br>							
		<div align="center" class="t1"><br><strong>IPStat <?=$this->ver?><br>Copyright &copy; 2003 - 2005 ������� ������, ����� ���������<br><a href="http://ipstat.perm.ru" class="t1">http://ipstat.perm.ru</a></strong></div><br>
		
		<?
	}
}
?>